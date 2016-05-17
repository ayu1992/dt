#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <random>
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

const int numCenters = 4000;
const int numCodebookIter = 8;
const int numFolds = 5;
const int numClasses = 13;
const int videoTrajectoryCap = 10000;	// this is actually videoTrajectoryCap
const int classTrajectoryCap = 100000;

/* TODO: This file will be deprecated and the functions will live in a library */
// TODO: Fine as is, but I would make actionlabel an enum in proto instead of string.
//       Saves all the hustle converting it back and forth between string and int.
const std::unordered_map<std::string, int> actionLabelLookUp ({
	{"BackGolf", 7},
	{"Diving", 0},
	{"FrontGolf", 1},
	{"FrontKick", 2},
	{"Horse", 8},
	{"Lifting", 3},
	{"Running", 6},
	{"SideGolf",4},
	{"SideKick",5},
	{"SideSwing", 9},
	{"Skateboard", 10},
	{"SwingBench", 11},
	{"Walking", 12}
});

// 5 fold cv data splits for a class
// ! vid refers to the index in videoList
/* This struct needs to be refactored */
struct DataSplit {
	// TODO: An std::array<std::set<int>, 5> would work here, since the size is compile-time constant. Fine as is.
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
std::map<int, std::vector<int>> parseVideoList(const motionClustering::VideoList& videoList) {

	std::map<int, std::vector<int>> usableVideos;

	for (int vid = 0; vid < videoList.videos().size(); ++vid) {
		const motionClustering::VideoInstance& this_video = videoList.videos(vid);

		if (this_video.tracks_size() > 0) {
			// Count how many usable videos there are for each action category
			usableVideos[actionLabelLookUp.at(this_video.actionlabel())].push_back(vid);
		}
	}
	return usableVideos;
}

int countTracks(const motionClustering::VideoList& videoList) {
	int numData = 0;
	for (const auto& v : videoList.videos()) {
  		numData += v.tracks_size();
  	}
  	std::cout << "I counted " << numData << "tracks" << std::endl;
  	return numData;
}

void generateCrossValidationSets(const std::map<int, std::vector<int>>& usableVideos, std::map<int, DataSplit>& dataSplitForClass) {

	for (const auto& actionClass : usableVideos) {
		int numVideos = actionClass.second.size();	// 40% training, 30% valid, 30% test
		int numTest = numVideos * 0.3;
		int numVal = numTest;
		int numTraining = numVideos - numTest - numVal;
		
		std::vector<int> totalVideos = actionClass.second;

		int classId = actionClass.first;
		// Set up test set and put aside
		for (int i = 0; i < numTest; ++i) {
			int j = i + (rand() % (totalVideos.size() - i));// random number in [i, list.size());
			dataSplitForClass[classId].TestingSetVid.insert(totalVideos[j]);	
			std::swap(totalVideos[i], totalVideos[j]);
		}		

		// Use remaining videos for training and validation
		std::vector<int> remainingVideos(totalVideos.begin() + numTest, totalVideos.end());

		dataSplitForClass[classId].TrainingSetVid.assign(numFolds, {});
		dataSplitForClass[classId].ValidationSetVid.assign(numFolds, {});
		dataSplitForClass[classId].TrainingSetSizes.assign(numFolds, 0);
		dataSplitForClass[classId].ValidationSetSizes.assign(numFolds, 0);

		// Training set and Validation set for five folds
		for (int f = 0; f < numFolds; ++f) {
			for (int i = 0; i < numTraining; ++i) {
				int j = i + (rand() % (remainingVideos.size() - i));
				dataSplitForClass[classId].TrainingSetVid[f].insert(remainingVideos[j]);
				std::swap(remainingVideos[i], remainingVideos[j]);
			}
			// Assign all residual videos to validation set
			dataSplitForClass[classId].ValidationSetVid[f].insert(remainingVideos.begin() + numTraining, remainingVideos.end());
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
  	for (const auto& pair : usableVideos) {
  		std::cout << "Class " << pair.first << " : "; 
  		for (int index : pair.second) {
  			std::cout << index << ", ";
  		}
  		std::cout << std::endl;
  	}
  	std::cout << "====================================================================" << std::endl;
  	return;
}

// Functor should have signature similar to this: const RepeatedField<float>& (const Trajectory&).
template <typename Functor>
void appendTracksToData(const google::protobuf::RepeatedPtrField< ::motionClustering::Trajectory>& tracks, int& row, float* data) {
	if (tracks.empty())	{
		std::cout << "returning.." << std::endl;
		return;
	}
	for (const auto& t : tracks) {;
		for (int i = 0; i < Functor::dimension; ++i) {
			data[row * Functor::dimension + i] = Functor()(t).Get(i); //t.normalizedpoints(i);
		}
		++row;
	}
}

template <typename Functor>
int makeCodebook(
	float* data, 
	const int numData, 
	std::vector<std::unique_ptr<VlKMeans>>& kmeansInstances, 
	std::vector<float *>& kmeansCenters,
	std::vector<float>& sumDistances) {

  	// Initialize kmeans 8 times and select result with lowest error
	for (int i = 0; i < numCodebookIter; ++i) {
	  	// Build codebook
	  	VlKMeans* kmeans = vl_kmeans_new(VL_TYPE_FLOAT, VlDistanceL2);
	  	vl_kmeans_set_algorithm (kmeans, VlKMeansLloyd) ;	
	  	std::cout << "Running kmeans to build codebook iter : "<< i << std::endl;
  		// Initialize the cluster centers by randomly sampling the data	  	
		vl_kmeans_init_centers_with_rand_data (kmeans, data, Functor::dimension, numData, numCenters) ;
		vl_kmeans_set_max_num_iterations (kmeans, 100);
		vl_kmeans_refine_centers (kmeans, data, numData) ;

  		std::unique_ptr<vl_uint32[]> assignments(new vl_uint32[numData]);
		std::unique_ptr<float[]> distances(new float[numData]);

		vl_kmeans_quantize(kmeans, assignments.get(), distances.get(), data, numData);
  		
  		kmeansInstances.emplace_back(kmeans);
		kmeansCenters.push_back((float*)vl_kmeans_get_centers(kmeans));
		
		float sum = 0.0;
		for (size_t i = 0; i < numData; i++) {
			sum += distances[i];
		}
		
		sumDistances.push_back(sum);
  	}
  	std::cout << "Codebook complete" << std::endl;
	 // Use the centers with lowest sum of distances as codebook
	 auto lowestDistance_it = std::min_element(sumDistances.begin(), sumDistances.end());
	 return std::distance(sumDistances.begin(), lowestDistance_it);
}

// Returns a histogram for a video, featureVector: 1(video) x 4000
std::vector<float> transformData(VlKMeans* kmeans, float* data, int numTracks) {
	std::unique_ptr<vl_uint32[]> assignments(new vl_uint32[numTracks]);
	std::unique_ptr<float[]> distances(new float[numTracks]);

	std::vector<float> featureVector(numCenters, 0.0);

	vl_kmeans_quantize(kmeans, assignments.get(), distances.get(), data, numTracks);
	for (int i = 0; i < numTracks; ++i) {
		int a = assignments[i];
		if (a >= featureVector.size()) {
			std::cout << "Invalid assignment: " << a << " from track #" << i << std::endl;
			continue;
		}
		featureVector[a] += static_cast<float>(1.0 / numTracks);	
	}
	return featureVector;		// rvo
}

void writeFeaturesToFile(
	const std::string& filepath, 
	const motionClustering::VideoList& videoList, 
	const std::map<int, std::vector<int>>& vidInSequence,			// class id -> vector of vid
	const std::vector<std::vector<float>>& points,
	const std::vector<std::vector<float>>& hog,
	const std::vector<std::vector<float>>& hof,
	const std::vector<std::vector<float>>& mbhx,
	const std::vector<std::vector<float>>& mbhy
	) {

	std::cout << "Writing features to file" << std::endl;
	std::ofstream fout;
	fout.open (filepath, std::fstream::in | std::fstream::out | std::fstream::app);
	auto write_to_file = [&fout](int row, int start_num, const std::vector<std::vector<float>>& feature){
		for (int i = 0; i < numCenters; ++i) fout << (i + start_num) << ":" << feature[row][i] << " ";
	};
	// Label, index(1...):value
	int row = 0;
	for (const auto& actionClass : vidInSequence) {
		for (int vid : actionClass.second) {
			fout << actionLabelLookUp.at(videoList.videos(vid).actionlabel()) << " ";
			write_to_file(row, 1, points);
			write_to_file(row, numCenters + 1, hog);
			write_to_file(row, numCenters * 2 + 1, hof);
			write_to_file(row, numCenters * 3 + 1, mbhx);
			write_to_file(row, numCenters * 4 + 1, mbhy);
			fout << std::endl;
			++row;	
		}
	}
	fout.close();
}

void writeSingleChannelToFile(
	const std::string& filepath,
	const motionClustering::VideoList& videoList,
	const std::map<int, std::vector<int>>& vidInSequence,
	const std::vector<std::vector<float>>& channel
	) {

	std::cout << "Writing channel to file" << std::endl;
	std::ofstream fout;
	fout.open(filepath, std::fstream::in | std::fstream::out | std::fstream::app);
	int row = 0;
	for (const auto& actionClass : vidInSequence) {
		for (int vid : actionClass.second) {
			fout << actionLabelLookUp.at(videoList.videos(vid).actionlabel()) << " ";
			for (int i = 0; i < numCenters; ++i) {
				fout << i + 1 << ":" << channel[row][i] << " ";
			}
			fout << std::endl;
			++row;
		}
	}
	fout.close();
}

// a, b are the same channel of two different samples.
float computeChiSquareDistance(const std::vector<float>& a, const std::vector<float>& b) {
	float chi_square = 0.0;
	auto it1 = a.begin();
	auto it2 = b.begin();
	for (; it1 != a.end(); ++it1, ++it2) {
		if (*it1 == 0 && *it2 == 0) continue;
		chi_square += ((*it1 - *it2) * (*it1 - *it2) / (*it1 + *it2));
	}
	return chi_square / 2;
}

std::vector<std::vector<float>> computeNormalizedChiSquareDistanceForChannel(const std::vector<std::vector<float>>& channel, float& Ac) {
	float sum = 0;
	const int N = channel.size();
	std::vector<std::vector<float>> ret(N, std::vector<float>(N));
	for (int i = 0; i < N; ++i) {
		for (int j = i; j < N; ++j) {
			float chi_sq = computeChiSquareDistance(channel[i], channel[j]);
			sum += chi_sq;
			ret[i][j] = chi_sq;
			if (i != j) {
				sum += chi_sq;
				ret[j][i] = chi_sq;
			}
		}
	}
	Ac = sum / (N * N);
	for (auto& v : ret) {
		for (float& f : v) f /= Ac;
	}
	return ret;
}

std::vector<std::vector<float>> computeNormalizedChiSquareDistanceForTesting(
	const std::vector<std::vector<float>>& testing,
	const std::vector<std::vector<float>>& training,
	float Ac) {
	const int M = testing.size();
	const int N = training.size();
	std::vector<std::vector<float>> ret(M, std::vector<float>(N));
	for (int i = 0; i < M; ++i) {
		for (int j = 0; j < N; ++j) ret[i][j] = computeChiSquareDistance(testing[i], training[j]) / Ac;
	}
	return ret;
}

// Returns Ac's
std::array<float, 5> writeTrainingKernelsToFile(
	const std::string& filepath,
	const motionClustering::VideoList& videoList, 
	const std::map<int, std::vector<int>>& vidInSequence,			// class id -> vector of vid
	const std::vector<std::vector<float>>& points,
	const std::vector<std::vector<float>>& hog,
	const std::vector<std::vector<float>>& hof,
	const std::vector<std::vector<float>>& mbhx,
	const std::vector<std::vector<float>>& mbhy) {
	const int N = points.size();
	std::vector<std::vector<float>> K(N, std::vector<float>(N, 0));
	std::array<float, 5> acs;

	auto add_channel_to_K = [&K, &acs, N](const std::vector<std::vector<float>>& channel, int channel_num) {
		std::vector<std::vector<float>> channel_chi_sq = computeNormalizedChiSquareDistanceForChannel(channel, acs[channel_num]);
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) K[i][j] += channel_chi_sq[i][j];
		}
	};

	add_channel_to_K(points, 0);
	add_channel_to_K(hog, 1);
	add_channel_to_K(hof, 2);
	add_channel_to_K(mbhx, 3);
	add_channel_to_K(mbhy, 4);
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j) K[i][j] = std::exp(-1 * K[i][j]);
	}

	std::cout << "Writing kernel to file" << std::endl;
	std::ofstream fout;
	fout.open (filepath, std::fstream::in | std::fstream::out | std::fstream::app);
	auto map_it = vidInSequence.begin();
	auto vec_it = map_it->second.begin();

	for (int i = 0; i < N; ++i) {
		fout << labels[i] << " ";		// label
		fout << "0:" << (i + 1);
		for (int j = 0; j < N; ++j) fout << " " << (j + 1) << ":" << K[i][j];
		fout << std::endl;

		++vec_it;
		while (vec_it == map_it->second.end()) {
			++map_it;
			vec_it = map_it->second.begin();
		}
	}

	fout.close();
	return acs;
}

void writeTestingKernelsToFile(
	const std::string& filepath,
	const motionClustering::VideoList& videoList, 
	const std::map<int, std::vector<int>>& vidInSequence,			// class id -> vector of vid
	const std::array<float, 5>& acs,
	const std::vector<std::vector<float>>& points_te,
	const std::vector<std::vector<float>>& points_tr,
	const std::vector<std::vector<float>>& hog_te,
	const std::vector<std::vector<float>>& hog_tr,
	const std::vector<std::vector<float>>& hof_te,
	const std::vector<std::vector<float>>& hof_tr,
	const std::vector<std::vector<float>>& mbhx_te,
	const std::vector<std::vector<float>>& mbhx_tr,
	const std::vector<std::vector<float>>& mbhy_te,
	const std::vector<std::vector<float>>& mbhy_tr) {
	const int M = points_te.size();  // Size of testing set.
	const int N = points_tr.size();  // Size of training set.
	std::vector<std::vector<float>> K(M, std::vector<float>(N, 0));

	auto add_channel_to_K = [&K, &acs, M, N](const std::vector<std::vector<float>>& testing, const std::vector<std::vector<float>>& training, int channel_num) {
		std::vector<std::vector<float>> channel_chi_sq = computeNormalizedChiSquareDistanceForTesting(testing, training, acs[channel_num]);
		for (int i = 0; i < M; ++i) {
			for (int j = 0; j < N; ++j) K[i][j] += channel_chi_sq[i][j];
		}
	};

	add_channel_to_K(points_te, points_tr, 0);
	add_channel_to_K(hog_te, hog_tr, 1);
	add_channel_to_K(hof_te, hof_tr, 2);
	add_channel_to_K(mbhx_te, mbhx_tr, 3);
	add_channel_to_K(mbhy_te, mbhx_tr, 4);
	for (int i = 0; i < M; ++i) {
		for (int j = 0; j < N; ++j) K[i][j] = std::exp(-1 * K[i][j]);
	}

	std::cout << "Writing kernel to file" << std::endl;
	std::ofstream fout;
	fout.open (filepath, std::fstream::in | std::fstream::out | std::fstream::app);
	auto map_it = vidInSequence.begin();
	auto vec_it = map_it->second.begin();
	// Avoid empty vectors.
	while (vec_it == map_it->second.end()) {
		++map_it;
		vec_it = map_it->second.begin();
	}

	for (int i = 0; i < M; ++i) {
		fout << map_it->first << " ";
		fout << "0:" << (i + 1);
		for (int j = 0; j < N; ++j) fout << " " << (j + 1) << ":" << K[i][j];
		fout << std::endl;

		++vec_it;
		while (vec_it == map_it->second.end()) {
			++map_it;
			vec_it = map_it->second.begin();
		}
	}

	fout.close();
}