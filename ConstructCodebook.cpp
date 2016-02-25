#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <time.h>
#include <unordered_map>
#include "dump.pb.h"


extern "C" {
  #include <vl/generic.h>
}
#include <vl/kmeans.h>

const int dimension = 30;
const int numCenters = 4000;

const std::unordered_map<std::string, int> actionLabelLookUp ({
	{"BackGolf", 0},
	{"Diving", 1},
	{"FrontGolf", 2},
	{"FrontKick", 3},
	{"Horse", 4},
	{"Lifting", 5},
	{"Running", 6},
	{"SideGolf", 7},
	{"SideKick", 8},
	{"SideSwing", 9},
	{"Skateboard", 10},
	{"SwingBench", 11},
	{"Walking", 12}
});

// 5 fold cv data splits for a class
struct DataSplit {
	std::vector<std::set<int>> TrainingSetVid;
	std::vector<std::set<int>> ValidationSetVid;
	std::set<int> TestingSetVid;
};

/*
 * videoList : contains video proto objects
 * data : an empty matrix initialized with 0.0s
 * Fills data row by row
 */
void ParseVideoListAndFillMatrix(const motionClustering::VideoList& videoList, float* data, int row, int col, std::map<int, std::vector<int>>& usableVideos) {
	int line = 0;
	int index = 0;
	for (auto& v : videoList.videos()) {
		// TODO: instead of keeping a separate int to track the item, just iterate by index. Something like:
		//     for (int line = 0; line < v.tracks_size(); ++line) { /* Do something with v.tracks(line) */ }
		
		// If this video contains tracks, it is usable (for training, validation or testing)
		// If it doesn't contain any tracks, we still reserve a space for it in data for simplicity, 
		// But it will not be used for training, validation or testing
		if (v.tracks_size() > 0) {
			// Count how many usable videos there are for each action category
			usableVideos[actionLabelLookUp.at(v.actionlabel())].push_back(index);
		}

		for (auto track : v.tracks()) {
			for (int j = 0 ; j < col; j++) {
				data[line * col + j] = track.normalizedpoints(j);
			}
		line++;
		}
		index++;
	}
	std::cout << "Filled matrix with " <<line << " lines" << std::endl;
}

void PrintMatrix(float* data, int row, int col) {
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			std::cout << data[i * col + j] << ", ";
		}
		std::cout << std::endl;
	}
}

void writeCodeBookToFile(std::string filepath, float* centers) {
	std::ofstream fout;
	fout.open(filepath.c_str());

	for (int i = 0; i < numCenters; i++) {
		for (int j = 0; j < dimension; j++) {
			fout << centers[ dimension * i + j] << " ";
		}
		fout << std::endl;
	}
	fout.close();
}

void getFeatureVectors(vl_uint32* assignments, float* features, const motionClustering::VideoList& videoList) {
	// Initialize the matrix, unelegantly
	for (int i = 0; i < videoList.videos_size(); i++) {		// Refactor!
		for (int j = 0; j < numCenters; j++) {
			features[i * numCenters + j] = 0.0;
		}
	}

	int index = 0;
	for (int video = 0; video < videoList.videos_size(); video++) {
		int numTracks = videoList.videos(video).tracks_size();
		for (int i = index; i < index + numTracks; i++) {
			// Build normalized histograms
			features[video * numCenters + assignments[i]] += static_cast<float>(1.0 / numTracks);			// Possible bug?
		}
		index += numTracks;
	}
}

void writeFeaturesToFile(std::string filepath, float* features, const motionClustering::VideoList& videoList) {
	std::ofstream fTrainingSet;
	fTrainingSet.open(filepath.c_str());

	// Label, index(1...):value
	for (int video = 0; video < videoList.videos_size(); video++) {
		fTrainingSet << actionLabelLookUp.at(videoList.videos(video).actionlabel()) << " ";
		for (int i = 1; i <= numCenters; i++) {
			fTrainingSet << i << ":" << features[video * numCenters + (i - 1)] << " ";
		}
		fTrainingSet << std::endl;
	}
	fTrainingSet.close();
}

int countTracks(const motionClustering::VideoList& videoList) {
	int numData = 0;
	for (auto v : videoList.videos()) {
  		numData += v.tracks_size();
  	}
  	std::cout << "I counted " << numData << "tracks" << std::endl;
  	return numData;
}

void GenerateCrossValidationSets(const std::map<int, std::vector<int>>& usableVideos, std::map<int, DataSplit>& dataSplitForClass) {
	for (auto actionClass : usableVideos) {
		int numVideos = actionClass.second.size();	// 40% training, 30% valid, 30% test
		int numTest = numVideos * 0.3;
		int numVal = numTest;
		int numTraining = numVideos - numTest - numVal;
		
		std::vector<int>& totalVideos = actionClass.second;

		int classId = actionClass.first;
		// Set up test set and put aside
		for (int i = 0; i < numTest; ++i) {
			int j = i + (rand() % (totalVideos.size() - i));// random number in [i, list.size());
			dataSplitForClass[classId].TestingSetVid.insert(totalVideos[j]);		
			std::swap(totalVideos[i], totalVideos[j]);
		}		

		// Use remaining videos for training and validation
		std::vector<int> remainingVideos(totalVideos.begin() + numTest, totalVideos.end());
		dataSplitForClass[classId].TrainingSetVid.assign(5, {});
		dataSplitForClass[classId].ValidationSetVid.assign(5, {});

		// Training set and Validation set for five folds
		for (int fold = 0; fold < 5; ++fold) {
			for (int i = 0; i < numTraining; ++i) {
				int j = i + (rand() % (remainingVideos.size() - i));
				dataSplitForClass[classId].TrainingSetVid[fold].insert(remainingVideos[j]);
				std::swap(remainingVideos[i], remainingVideos[j]);
			}
			// Assign all residual videos to validation set
			dataSplitForClass[classId].ValidationSetVid[fold].insert(remainingVideos.begin() + numTraining, remainingVideos.end());
		}
	}
}
int main(int argc, char* argv[]) {

	srand(time(NULL));

	motionClustering::VideoList videoList;
	std::string filepath = argv[1];
	{
	    // Read the existing video list
	    std::fstream input( filepath + "TrajectoryDump.data", std::ios::in | std::ios::binary);
	    if (!videoList.ParseFromIstream(&input)) {
	      std::cerr << "Failed to parse TrajectoryDump.data" << std::endl;
	      return -1;
   		 }
  	}

  	// Count total number of tracks
	int numData = countTracks(videoList);	
	std::cout << "Training set contains " << numData << " trajectories" << std::endl;
	// Wraps around a float[numData * dimension].
	std::unique_ptr<float[]> data(new float[numData * dimension]);
	
	// action label -> usable video ids'
	std::map<int, std::vector<int>> usableVideos;
  	ParseVideoListAndFillMatrix(videoList, data.get(), numData, dimension, usableVideos);

  	std::cout << "Now printing out usable videos" << std::endl;
  	for (auto pair : usableVideos) {
  		std::cout << "Class " << pair.first << " : "; 
  		for (int index : pair.second) {
  			std::cout << index << ", ";
  		}
  		std::cout << std::endl;
  	}
  	/**
     * Partition data dump into training set, validation set and testing set
  	 */
  	// class label -> DataSplit
    std::map<int, DataSplit> dataSplitForClass;
    GenerateCrossValidationSets(usableVideos, dataSplitForClass);

  	/* Randomly select 100,000 training features*/
  	const int numCodebookIter = 8;
  	//std::vector<VLKMeans> kmeansInstances;
  	//std::vector<int> sumDistances;

  	/** Initialize kmeans 8 times and select result with lowest error **/
  	//for (int i = 0; i < numCodebookIter; i++) {
	  	// Build codebook
	  	VlKMeans* kmeans = vl_kmeans_new(VL_TYPE_FLOAT, VlDistanceL2);
	  	vl_kmeans_set_algorithm (kmeans, VlKMeansLloyd) ;	
	  	std::cout << "Running kmeans to build codebook" << std::endl;
  		// Initialize the cluster centers by randomly sampling the data	  	
		vl_kmeans_init_centers_with_rand_data (kmeans, data.get(), dimension, numData, numCenters) ;
		vl_kmeans_set_max_num_iterations (kmeans, 1000);
		vl_kmeans_refine_centers (kmeans, data.get(), numData) ;

		// The codebook!
		float* centers = (float*)vl_kmeans_get_centers(kmeans);
		//kmeansInstances.push_back(kmeans);
  	//}

	// Construct feature vectors for training set
	vl_uint32 assignments[numData];	//vl_uint32* assignments = vl_malloc(sizeof(vl_uint32) * numData);
	float distances[numData];	//float * distances = vl_malloc(sizeof(float) * numData);
	
	vl_kmeans_quantize(kmeans, assignments, distances, data.get(), numData);

	/*for (int i =0; i < numData; i++) {
			assignments[i] = 0;
			distances[i] = 0.0;
	}*/

	/*
	// print assignments
	std::ofstream assT;
	assT.open("AssignmentsTraining.txt");
	for (int i = 0; i < numData; i++) {		
		assT << assignments[i] << std::endl;
	}*/

	// number of videos x 4000
	std::unique_ptr<float[]> features(new float[videoList.videos_size() * numCenters]); //	float features[videoList.videos_size()][numCenters];
	getFeatureVectors(assignments, features.get(), videoList);

	// Output training set
	std::cout << "Writing training set features to file" << std::endl;
	std::string trainingSetPath = filepath + "TrainingSet.data";
	writeFeaturesToFile(trainingSetPath, features.get(), videoList);

	// delete videoList?

	// Read testing trajectory dump
	motionClustering::VideoList testSetVideoList;

	// Construct testing set
	/*
	{
		// Read the existing video list
		std::fstream testSetInput( filepath + "TestingSetTrajectoryDump.data", std::ios::in | std::ios::binary);
		if (!testSetVideoList.ParseFromIstream(&testSetInput)) {
		   std::cerr << "Failed to parse TestingTrajectoryDump.data" << std::endl;
		   return -1;
	   	}
	}

	numData = countTracks(testSetVideoList);
	std::cout << "Test set contains " << numData << " trajectories" << std::endl;
	std::unique_ptr<float[]> testData(new float[numData * dimension]);
  	ParseVideoListAndFillMatrix(testSetVideoList, testData.get(), numData, dimension);
  
  	// Construct feature vectors for testing set
	vl_uint32 testSetAssignments[numData];	//vl_uint32* assignments = vl_malloc(sizeof(vl_uint32) * numData);
	float testSetDistances[numData];	//float * distances = vl_malloc(sizeof(float) * numData);
	vl_kmeans_quantize(kmeans, testSetAssignments, testSetDistances, testData.get(), numData);

	// print assignments
	std::ofstream assTest;
	assTest.open("AssignmentsTesting.txt");
	for (int i = 0; i < numData; i++) {		
		assTest << assignments[i] << std::endl;
	}

	std::unique_ptr<float[]> testSetFeatures(new float[testSetVideoList.videos_size() * numCenters]);
	getFeatureVectors(testSetAssignments, testSetFeatures.get(), testSetVideoList);

	// Output training set
	std::cout << "Writing test set features to file" << std::endl;
	std::string testSet = filepath + "TestSet.data";
	writeFeaturesToFile(testSet, testSetFeatures.get(), testSetVideoList);
*/
  	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}