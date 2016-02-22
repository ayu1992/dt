#include <iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <unordered_map>
#include "dump.pb.h"

extern "C" {
  #include <vl/generic.h>
}
#include <vl/kmeans.h>

const int dimension = 30;
const int numCenters = 4000;

const std::unordered_map<std::string, int> actionLabelLookUp ({
	{"Diving", 0},
	{"Lifting", 1},
	{"Running", 2}
});

void ParseVideoListAndFillMatrix(const motionClustering::VideoList& videoList, float* data, int row, int col) {
	int line = 0;

	for (auto& v : videoList.videos()) {
		// TODO: instead of keeping a separate int to track the item, just iterate by index. Something like:
		//     for (int line = 0; line < v.tracks_size(); ++line) { /* Do something with v.tracks(line) */ }
		for (auto track : v.tracks()) {
			for (int j = 0 ; j < col; j++) {
				data[line * col + j] = track.normalizedpoints(j);
			}
		line++;
		}
	}
	std::cout << "Filled matrix with " <<line << " lines" << std::endl;
}

void writeFeatures() {}

/*
 * Reads "TrainingSetTrajectoryDump.data" and "TestingSetTrajecotryDump.data"
 * Construct codebook from training data
 * Output "TrainingSet.data" and "TestingSet.data"
 */
int main(int argc, char* argv[]) {
	motionClustering::VideoList videoList;
	std::string dumpPath = argv[1];
	std::string inputPath = dumpPath + "TrainingSetTrajectoryDump.data";

	{
	    // Read the existing video list
	    std::fstream input( inputPath, std::ios::in | std::ios::binary);
	    if (!videoList.ParseFromIstream(&input)) {
	      std::cerr << "Failed to parse TrainingSetTrajectoryDump.data" << std::endl;
	      return -1;
   		 }
  	}

  	/* Move this into helper function? */
	int numData = 0;	

  	for (auto v : videoList.videos()) {
  		numData += v.tracks_size();
  	}

	float data[numData][dimension];
  	ParseVideoListAndFillMatrix(videoList, (float*) data, numData, dimension);

  	VlKMeans* kmeans = vl_kmeans_new(VL_TYPE_FLOAT, VlDistanceL2);
  	vl_kmeans_set_algorithm (kmeans, VlKMeansLloyd) ;

  	// Initialize the cluster centers by randomly sampling the data
	vl_kmeans_init_centers_with_rand_data (kmeans, data, dimension, numData, numCenters) ;
	vl_kmeans_set_max_num_iterations (kmeans, 100);
	vl_kmeans_refine_centers (kmeans, &data, numData) ;

	// The codebook!
	float* centers = (float*)vl_kmeans_get_centers(kmeans);

	// Output Codebook (4000 x 30, seperated by space character)
	std::cout << "Writing codebook to file" <<  std::endl;
	std::ofstream fout;
	std::string codeBookPath = dumpPath + "Codebook.data";

	fout.open(codeBookPath.c_str());

	for (int i = 0; i < numCenters; i++) {
		for (int j = 0; j < dimension; j++) {
			fout << centers[ dimension * i + j] << " ";
		}
		fout << std::endl;
	}
	fout.close();

	// Construct feature vectors for training set
	vl_uint32 assignments[numData];	//vl_uint32* assignments = vl_malloc(sizeof(vl_uint32) * numData);
	float distances[numData];	//float * distances = vl_malloc(sizeof(float) * numData);
	vl_kmeans_quantize(kmeans, assignments, distances, data, numData);

	// number of videos x 4000
	float features[videoList.videos_size()][numCenters];
	// Initialize the matrix, unelegantly
	for (int i = 0; i < videoList.videos_size(); i++) {		// Refactor!
		for (int j = 0; j < numCenters; j++) {
			features[i][j] = 0.0;
		}
	}

	int index = 0;
	for (int video = 0; video < videoList.videos_size(); video++) {
		int numTracks = videoList.videos(video).tracks_size();
		for (int i = index; i < index + numTracks; i++) {
			// Build normalized histograms
			features[video][assignments[i]] += static_cast<float>(1.0 / numTracks);			// Possible bug?
		}
		index += numTracks;
	}

	// Output training set
	std::cout << "Writing training set features to file" << std::endl;
	std::ofstream fTrainingSet;
	std::string trainingSet = dumpPath + "TrainingSet.data";
	fTrainingSet.open(trainingSet.c_str());

	// Label, index(1...):value
	for (int video = 0; video < videoList.videos_size(); video++) {
		fTrainingSet << actionLabelLookUp.at(videoList.videos(video).actionlabel()) << " ";
		// TODO: make 4000 a constant in this file instead of hardcoding it everywhere.
		for (int i = 1; i <= numCenters; i++) {
			fTrainingSet << i << ":" << features[video][i - 1] << " ";
		}
		fTrainingSet << std::endl;
	}
	fTrainingSet.close();

	// Read testing trajectory dump
	std::string testingDump = argv[2];
	std::string testingPath = testingDump + "TestSetTrajectoryDump.data";
	motionClustering::VideoList testSetVideoList;

	// Construct testing set
	{
		    // Read the existing video list
		    std::fstream testSetInput( testingPath, std::ios::in | std::ios::binary);
		    if (!testSetVideoList.ParseFromIstream(&testSetInput)) {
		      std::cerr << "Failed to parse TestingTrajectoryDump.data" << std::endl;
		      return -1;
	   		 }
	}
	numData = ... 
	float testData[numData][dimension];
  	ParseVideoListAndFillMatrix(testSetVideoList, (float*) testData, numData, dimension);
  	// Construct feature vectors for testing set
	vl_uint32 testSetAssignments[numData];	//vl_uint32* assignments = vl_malloc(sizeof(vl_uint32) * numData);
	float distances[numData];	//float * distances = vl_malloc(sizeof(float) * numData);
	vl_kmeans_quantize(kmeans, testSetAssignments, distances, data, numData);

	// number of videos x 4000
	float testSetFeatures[testSetVideoList.videos_size()][numCenters];
	// Initialize the matrix, unelegantly
	for (int i = 0; i < testSetVideoList.videos_size(); i++) {		// Refactor!
		for (int j = 0; j < numCenters; j++) {
			testSetFeatures[i][j] = 0.0;
		}
	}

	int index = 0;
	for (int video = 0; video < testSetVideoList.videos_size(); video++) {
		int numTracks = testSetVideoList.videos(video).tracks_size();
		for (int i = index; i < index + numTracks; i++) {
			// Build normalized histograms
			testSetFeatures[video][testSetAssignments[i]] += static_cast<float>(1.0 / numTracks);			// Possible bug?
		}
		index += numTracks;
	}

	// Output training set
	std::cout << "Writing test set features to file" << std::endl;
	std::ofstream fTestSet;
	std::string testSet = dumpPath + "TestSet.data";
	fTestSet.open(testSet.c_str());
	// Label, index(1...):value
	for (int video = 0; video < testSetVideoList.videos_size(); video++) {
		fTestSet << actionLabelLookUp.at(testSetVideoList.videos(video).actionlabel()) << " ";
		for (int i = 1; i <= numCenters; i++) {
			fTestSet << i << ":" << testSetFeatures[video][i - 1] << " ";
		}
		fTestSet << std::endl;
	}
	fTestSet.close();

  	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}

/*for(int i = 0; i < videoList.videos_size(); i++) {
  		std::cout << "Video : " << i << std::endl;
  		const motionClustering::VideoInstance&  video = videoList.videos(i);
  		std::cout << "Action Category : " << video.actionlabel() << std::endl;
  		std::cout << "Video index : " << video.videoindex() << std::endl;
  		std::cout << "Num clusters : " << video.numclusters() << std::endl;
  		std::cout << "Tracks size : " << video.tracks_size() << std::endl;
  		std::cout << "First track contains : " << video.tracks(0).normalizedpoints_size() << "pairs of coords" << std::endl;
  		std::cout << "Second track contains : " << video.tracks(1).normalizedpoints_size() << "pairs of coords" << std::endl;
  	}*/