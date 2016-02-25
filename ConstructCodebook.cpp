#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
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
const int numCodebookIter = 2;

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
// ! vid refers to the index in videoList
struct DataSplit {
	std::vector<std::set<int>> TrainingSetVid;
	std::vector<int> TrainingSetSizes;			// number of tracks for each fold
	std::vector<std::set<int>> ValidationSetVid;
	std::vector<int> ValidationSetSizes;
	std::set<int> TestingSetVid;
};
/*
 * videoList : contains video proto objects
 * data : an empty matrix initialized with 0.0s
 * Fills data row by row
 * ! videoList contains videos with 0 tracks
 * ! usableVideos only contains videos with > 0 tracks
 */
void parseVideoList(const motionClustering::VideoList& videoList, std::map<int, std::vector<int>>& usableVideos) {

	for (int vid = 0; vid < videoList.videos().size(); ++vid) {
		const motionClustering::VideoInstance& this_video = videoList.videos(vid);

		if (this_video.tracks_size() > 0) {
			// Count how many usable videos there are for each action category
			usableVideos[actionLabelLookUp.at(this_video.actionlabel())].push_back(vid);
		}
/*
		for (auto track : this_video.tracks()) {
			for (int j = 0 ; j < col; j++) {
				data[line * col + j] = track.normalizedpoints(j);
			}*/
	}
}

int countTracks(const motionClustering::VideoList& videoList) {
	int numData = 0;
	for (auto v : videoList.videos()) {
  		numData += v.tracks_size();
  	}
  	std::cout << "I counted " << numData << "tracks" << std::endl;
  	return numData;
}

void generateCrossValidationSets(const std::map<int, std::vector<int>>& usableVideos, std::map<int, DataSplit>& dataSplitForClass) {
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
		dataSplitForClass[classId].TrainingSetSizes.assign(5, 0);
		dataSplitForClass[classId].ValidationSetSizes.assign(5, 0);

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

void countTrackSizes(std::map<int, DataSplit>& dataSplitForClass, const motionClustering::VideoList& videoList) {
	for (int fold = 0; fold < 5; ++fold) {
		for (auto& actionClass : dataSplitForClass) {
			DataSplit& dataSplit = actionClass.second;
		
			std::set<int>& trainingvids = dataSplit.TrainingSetVid[fold];
			std::set<int>& validationvid = dataSplit.ValidationSetVid[fold];
			
			for (int vid : trainingvids) {
				dataSplit.TrainingSetSizes[fold] += videoList.videos(vid).tracks_size();
			}
			for (int vid : validationvid) {
				dataSplit.ValidationSetSizes[fold] += videoList.videos(vid).tracks_size();
			}
		}		
	}
}

void printUsableVideos(const std::map<int, std::vector<int>>& usableVideos) {
	std::cout << "Now printing out usable videos" << std::endl;
  	for (auto pair : usableVideos) {
  		std::cout << "Class " << pair.first << " : "; 
  		for (int index : pair.second) {
  			std::cout << index << ", ";
  		}
  		std::cout << std::endl;
  	}
  	std::cout << "====================================================================" << std::endl;
  	return;
}

void getFeatureVectors(vl_uint32* assignments, float* features, const motionClustering::VideoList& videoList, const std::vector<int>& vidInSequence) {
	// Initialize the matrix, unelegantly
	std::cout << "getFeatureVectors" << std::endl;
	for (int i = 0; i < vidInSequence.size(); i++) {		// Refactor!
		for (int j = 0; j < numCenters; j++) {
			features[i * numCenters + j] = 0.0;
		}
	}

	int index = 0;
	// n videos -> features will be n x 4000
	for (int row = 0; row < vidInSequence.size(); row++) {		// For each row in features
		int vid = vidInSequence[row];							// reference of this video in videoList
		int numTracks = videoList.videos(vid).tracks_size();
		for (int i = index; i < index + numTracks; i++) {
			// Build normalized histograms
			features[row * numCenters + assignments[i]] += static_cast<float>(1.0 / numTracks);
		}
		index += numTracks;
	}
}

void writeFeaturesToFile(std::string filepath, float* features, const motionClustering::VideoList& videoList, const std::vector<int>& vidInSequence) {
	std::cout << "Writing features to file" << std::endl;
	std::ofstream fTrainingSet;
	fTrainingSet.open(filepath.c_str());

	// Label, index(1...):value
	for (int row = 0; row < vidInSequence.size(); row++) {
		fTrainingSet << actionLabelLookUp.at(videoList.videos(vidInSequence[row]).actionlabel()) << " ";
		for (int i = 1; i <= numCenters; i++) {
			fTrainingSet << i << ":" << features[row * numCenters + (i - 1)] << " ";
		}
		fTrainingSet << std::endl;
	}
	fTrainingSet.close();
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

	std::cout << "Total data dump contains " << countTracks(videoList) << " trajectories" << std::endl;

	// action label -> indices in videoList (videos with > 0 tracks only)
	std::map<int, std::vector<int>> usableVideos;

  	parseVideoList(videoList, usableVideos);

 	printUsableVideos(usableVideos);

    // Partition data into training set, validation set and testing set
    std::map<int, DataSplit> dataSplitForClass;   	// action label -> DataSplit

    generateCrossValidationSets(usableVideos, dataSplitForClass);

    countTrackSizes(dataSplitForClass, videoList);

  	// Count total number of tracks are going to be used for codebook
	int numData = 0;
	for (const auto& actionClass : dataSplitForClass) {
		numData += actionClass.second.TrainingSetSizes[0];
		numData += actionClass.second.ValidationSetSizes[0];
	}

	// Wraps around a float[numData * dimension].
	std::unique_ptr<float[]> data(new float[numData * dimension]);

	// Fill up data
	int row = 0;
	for (const auto& actionClass : dataSplitForClass) {
		// Use the data of the 1st fold
		for (int vid : actionClass.second.TrainingSetVid[0]) {
			const google::protobuf::RepeatedPtrField< ::motionClustering::Trajectory >& tracks = videoList.videos(vid).tracks();
			for (const auto& t : tracks) {
				for (int i = 0; i < dimension; ++i) {
					data[row * dimension + i] = t.normalizedpoints(i);
				}
				row++;
			}
		}

		for (int vid : actionClass.second.ValidationSetVid[0]) {
			const google::protobuf::RepeatedPtrField< ::motionClustering::Trajectory >& tracks = videoList.videos(vid).tracks();
			for (const auto& t : tracks) {
				for (int i = 0; i < dimension; ++i) {
					data[row * dimension + i] = t.normalizedpoints(i);
				}
				row++;
			}
		}
	}
    std::cout << row << " tracks were used for codebook" << std::endl;

  	/* Randomly select 100,000 training features*/
	
	std::vector<VlKMeans> kmeansInstances;
	std::vector<float *> kmeansCenters;
  	std::vector<int> sumDistances;
  	std::vector<vl_uint32* > kmeansAssignments;

  	// Initialize kmeans 8 times and select result with lowest error
  	for (int i = 0; i < numCodebookIter; i++) {
	  	// Build codebook
	  	VlKMeans* kmeans = vl_kmeans_new(VL_TYPE_FLOAT, VlDistanceL2);
	  	vl_kmeans_set_algorithm (kmeans, VlKMeansLloyd) ;	
	  	std::cout << "Running kmeans to build codebook iter : "<< i << std::endl;
  		// Initialize the cluster centers by randomly sampling the data	  	
		vl_kmeans_init_centers_with_rand_data (kmeans, data.get(), dimension, numData, numCenters) ;
		vl_kmeans_set_max_num_iterations (kmeans, 1000);
		vl_kmeans_refine_centers (kmeans, data.get(), numData) ;

  		vl_uint32* assignments = new vl_uint32[numData];
		float* distances = new float[numData];

		vl_kmeans_quantize(kmeans, assignments, distances, data.get(), numData);
  		
  		kmeansInstances.push_back(*kmeans);
		kmeansCenters.push_back((float*)vl_kmeans_get_centers(kmeans));
		
		kmeansAssignments.push_back(assignments);
		delete[] assignments;
		
		float sum = 0.0;
		for (size_t i = 0; i < numData; i++) {
			sum += distances[i];
		}
		
		sumDistances.push_back(sum);
		delete[] distances;
  	}

	 // Use the centers with lowest sum of distances as codebook
	 auto lowestDistance_it = std::min_element(sumDistances.begin(), sumDistances.end());
	 int codebook_index = std::distance(sumDistances.begin(), lowestDistance_it);

	 /* Generate 5 training sets, 5 validation sets and 1 testing set */
	 for (int fold = 0; fold < 5; ++fold) {

	 	/** GENERATE TRAINING SETS **/
		numData = 0;
		int numTrainingSetVideos = 0;
		for (const auto& actionClass : dataSplitForClass) {
			numData += actionClass.second.TrainingSetSizes[fold];
			numTrainingSetVideos += actionClass.second.TrainingSetVid[fold].size();
		}

		std::unique_ptr<float[]> trainingData(new float[numData * dimension]);

		// Fill up trainingData
		int row = 0;
		std::vector<int> vidInSequence;
		for (const auto& actionClass : dataSplitForClass) {
			for (int vid : actionClass.second.TrainingSetVid[fold]) {
				vidInSequence.push_back(vid);
				const google::protobuf::RepeatedPtrField< ::motionClustering::Trajectory >& tracks = videoList.videos(vid).tracks();
				for (const auto& t : tracks) {
					for (int i = 0; i < dimension; ++i) {
						trainingData[row * dimension + i] = t.normalizedpoints(i);
					}
					row++;
				}
			}
		}

		vl_uint32* assignments = new vl_uint32[numData];
		float* distances = new float[numData];

		vl_kmeans_quantize(&kmeansInstances[codebook_index], assignments, distances, trainingData.get(), numData);

		// number of videos x 4000
		std::unique_ptr<float[]> features(new float[numTrainingSetVideos * numCenters]);

		getFeatureVectors(assignments, features.get(), videoList, vidInSequence);

		delete[] assignments;
		delete[] distances;

		// Output training set
		std::cout << "Writing training set features to file" << std::endl;
		std::string trainingSetPath = filepath + "TrainingSet" + std::to_string(fold) + ".data";
		writeFeaturesToFile(trainingSetPath, features.get(), videoList, vidInSequence);
	}

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