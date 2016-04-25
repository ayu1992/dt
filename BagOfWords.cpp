#include <limits>       // std::numeric_limits
#include <random>
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand
#include "BoostRelatedHelpers.h"
#include <dirent.h>
extern "C" {
  #include <vl/generic.h>
}
#include <vl/kmeans.h>

constexpr int kNumRandomSamples = 100000;
constexpr int reservoirCacheSize = 300000;
constexpr int numCenters = 4000;
constexpr int numCodebookIter = 8;

using ClassToFileNames = std::map<std::string, std::vector<std::string>>;
using LabelAndTracks = std::pair<int, std::vector<track>>;

/* Reads and load a bunch of archives (from a specified location) in memory and output feature vectors */
std::vector<std::string> getArchiveNames(const std::string archivesLocation) {
	std::vector<std::string> archiveNames;
	DIR* dirp = opendir(archivesLocation.c_str());
	dirent* dp = NULL;
	while ((dp = readdir(dirp)) != NULL) {
		std::string name = dp->d_name;
		if(name.length() > 2) {
			archiveNames.push_back(name);
		}
	}
	closedir(dirp);
	return archiveNames;
}

template <typename T>
void reservoirSample(std::vector<T>& samples, int num_samples, int j, const T& t) {
	if (j < num_samples - 1) {
		samples[j] = t;
		return;
	}
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, j);
	int i = dis(gen);
	if (i >= num_samples) return;
	samples[i] = t;
}

void getTrainingSamples(std::vector<track>& samples, const std::vector<std::string>& archiveNames, const std::string archivesLocation) {
	int j = 0;
	for (const std::string& path : archiveNames) {
		videoRep video;
		restoreVideoRep(archivesLocation + path, video);
		for (const auto& id_and_track : video.largestCluster().tracks()) {
			reservoirSample(samples, kNumRandomSamples, j++, id_and_track.second);
			if (j % reservoirCacheSize == 0) std::cout << "j: " << j << std::endl; 
		}
	}
}

void checkContainNaN(const std::vector<track>& tracks) {
	std::cout << "Checking for dirty data" << std::endl;
	bool isDirty;
	for (const auto& t : tracks) {
		isDirty = false;
		std::for_each(t.displacements.begin(), t.displacements.end(), [&isDirty](const point& p){
			if (!std::isfinite(p.x) || !std::isfinite(p.y))	isDirty = true;
		});

		if(std::any_of(t.hog.begin(), t.hog.end(), [](const float& f){return !std::isfinite(f);})
		|| std::any_of(t.hof.begin(), t.hof.end(), [](const float& f){return !std::isfinite(f);})
		|| std::any_of(t.mbhx.begin(), t.mbhx.end(), [](const float& f){return !std::isfinite(f);})
		|| std::any_of(t.mbhy.begin(), t.mbhy.end(), [](const float& f){return !std::isfinite(f);})) {
			isDirty = true;
		}
		if (isDirty) std::cout << "===========================Dirty data============================" << std::endl;
	}
}

template <typename Functor>
std::unique_ptr<float[]> getData (const std::vector<track>& tracks) {
	std::unique_ptr<float[]> data(new float[tracks.size() * Functor::dimension]);
	int row = 0;
	for (const auto& t : tracks) {
		const std::vector<float>& track_data = Functor()(t);
		for (int i = 0; i < Functor::dimension; ++i) {
			data[row * Functor::dimension + i] = track_data[i];
		}
		++row;
	}
	return data;
}

template <typename Functor>
std::unique_ptr<VlKMeans> makeCodebook(float* data, const int numData) {
	std::cout << "Making codebook" << std::endl;
	std::vector<std::unique_ptr<VlKMeans>> kmeansInstances;
	std::vector<float *> kmeansCenters;	
  	std::vector<float> sumDistances;
  	double energy;
  	// Initialize kmeans 8 times and select result with lowest error
	for (int i = 0; i < numCodebookIter; ++i) {
	  	// Build codebook
	  	VlKMeans* kmeans = vl_kmeans_new(VL_TYPE_FLOAT, VlDistanceL2);
	  	vl_kmeans_set_algorithm (kmeans, VlKMeansLloyd) ;	
	  	std::cout << "Running kmeans to build codebook iter : "<< i << std::endl;
  		// Initialize the cluster centers by randomly sampling the data	  	
		vl_kmeans_init_centers_with_rand_data (kmeans, data, Functor::dimension, numData, numCenters) ;
		vl_kmeans_set_max_num_iterations (kmeans, 1000);
		vl_kmeans_refine_centers (kmeans, data, numData) ;
		energy = vl_kmeans_get_energy(kmeans) ;

		vl_uint32* assignments = (vl_uint32*)vl_malloc(sizeof(vl_uint32) * numData);
		float* distances = (float*)vl_malloc(sizeof(float) * numData);
		
		vl_kmeans_quantize(kmeans, assignments, distances, data, numData);
  		
  		kmeansInstances.emplace_back(kmeans);
		kmeansCenters.push_back((float*)vl_kmeans_get_centers(kmeans));
		
		float sum = 0.0;
		for (size_t i = 0; i < numData; i++) {
			sum += distances[i];
		}
		
		sumDistances.push_back(sum);
  	}
	// Use the centers with lowest sum of distances as codebook
	auto lowestDistance_it = std::min_element(sumDistances.begin(), sumDistances.end());
	int index = std::distance(sumDistances.begin(), lowestDistance_it);
	return std::move(kmeansInstances[index]);
}

template <typename Functor>
std::unique_ptr<VlKMeans> getCodebook(const std::vector<track>& trainingSet) {
	checkContainNaN(trainingSet);
	std::unique_ptr<float []> data = getData<Functor>(trainingSet);
	return makeCodebook<Functor>(data.get(), kNumRandomSamples);	
}

std::vector<LabelAndTracks> getVideoLabelToTracks(const std::vector<std::string>& filenames, const std::string& pathToFiles) {
	std::vector<LabelAndTracks> ret;
	for (const std::string& filename : filenames) {
		videoRep video;
		restoreVideoRep(pathToFiles + filename, video);
		// Get rid of trajectory indices
		std::vector<track> temp;
		for (const auto& pair : video.largestCluster().tracks()) {
			temp.push_back(pair.second);
		}
		ret.emplace_back(video.classLabel(), temp);		
	}
	return ret;
}

inline float computeDistance(const std::vector<float>& data, const float* center, int dimension) {
	float distance = 0;
	for (int i = 0; i < dimension; ++i) distance += (data[i] - center[i]) * (data[i] - center[i]);
	return std::isfinite(distance) ? distance : std::numeric_limits<float>::max();
}

template <typename Functor>
std::vector<int> quantize(VlKMeans* kmeans, const std::vector<track>& tracks) {
	std::vector<int> assignments(tracks.size(), -1);
	int ti = 0;
	for (size_t ti = 0; ti < tracks.size(); ++ti) {
		const track& t = tracks[ti];
		float* centers = static_cast<float*>(kmeans->centers);						// centers is a 1D
		const std::vector<float>& track_data = Functor()(t);		// ex. 96 x 1
		float best_distance = computeDistance(track_data, centers, Functor::dimension);
		int best_index = 0;
		for (int i = 1; i < numCenters; ++i) {
			centers += Functor::dimension;
			float distance = computeDistance(track_data, centers, Functor::dimension);
			if (distance < best_distance) {
				best_distance = distance;
				best_index = i;
			}
		}
		assignments[ti] = best_index;
	}
	return assignments;
}

template <typename Functor>
std::vector<float> transformData(VlKMeans* kmeans, const std::vector<track>& tracks) {
	std::vector<int> assignments = quantize<Functor>(kmeans, tracks);
	std::vector<float> features(numCenters, 0);
	for (int i = 0; i < assignments.size(); ++i) {
		if (assignments[i] >= numCenters || assignments[i] < 0) {
			std::cout << "Bad assignment: " << assignments[i] << std::endl;
			continue;
		}
		features[assignments[i]] += 1;
	}
	for (size_t i = 0; i < features.size(); ++i) features[i] /= tracks.size();
	return features;
}

void writeFeaturesToFile(const std::string& filename, const std::multimap<int, std::vector<float>>& features) {
	std::cout << "Writing" << std::endl;
	std::ofstream fout;
	fout.open (filename, std::fstream::in | std::fstream::out | std::fstream::app);
	for (const auto& instance : features) {
		fout << instance.first;
		for (int i = 0; i < instance.second.size(); ++i) {
			fout << " " << i + 1 << ":" << instance.second[i];
		}
		fout << std::endl;
	}
	fout.close();
}

// Insert all items in `a` to b.end().
template <typename T>
void vectorInsert(const std::vector<T>& a, std::vector<T>& b) {
	b.insert(b.end(), a.begin(), a.end());
}

void generateFeatures(const std::vector<std::string>& filenames, const std::string& archivesLocation, const std::vector<std::unique_ptr<VlKMeans>>& codebooks, const std::string& outputName) {
	// transform data to features
	std::vector<LabelAndTracks> videos = getVideoLabelToTracks(filenames, archivesLocation);
	std::multimap<int, std::vector<float>> outputFile;	// label -> feaure

	for (const auto& instance : videos) {	// Each instance corresponds to a video
		if (instance.second.empty())	continue;
		std::cout << instance.second.size() << " tracks" << std::endl;
		std::cout << "Displacements" << std::endl;
		std::vector<float> feature = transformData<DisplacementsGetter>(codebooks[0].get(), instance.second);
		std::cout << "Hog" << std::endl;
		//std::vector<float> feature = transformData<HogGetter>(codebooks[0].get(), instance.second);
		vectorInsert(transformData<HogGetter>(codebooks[1].get(), instance.second), feature);
		std::cout << "Hof" << std::endl;
		vectorInsert(transformData<HofGetter>(codebooks[2].get(), instance.second), feature);
		std::cout << "MbhX" << std::endl;
		vectorInsert(transformData<MbhXGetter>(codebooks[3].get(), instance.second), feature);
		std::cout << "MbhY" << std::endl;
		vectorInsert(transformData<MbhYGetter>(codebooks[4].get(), instance.second), feature);
		// Write a line to output
		outputFile.emplace(instance.first, feature);
	}
	writeFeaturesToFile(outputName, outputFile);
	return;
}

int main(int argc, char** argv) {
	// Takes in a grid config
	// Get all archive names in the specified folder
	std::string archivesLocation = argv[1];

	std::string trainingSetPath = archivesLocation + "/all/";
	std::vector<std::string> trainingSetNames = getArchiveNames(trainingSetPath);

	// Randomly sample some trajectories 
	std::vector<track> samples(kNumRandomSamples);

	getTrainingSamples(samples, trainingSetNames, trainingSetPath);

	/* TODO: impl 30 channels*/

	// Compute codebook from samples
	std::vector<std::unique_ptr<VlKMeans>> codebooks;
	// Split the samples

	// Given some tracks, compute 5 x 4000
	std::cout << "Compute codebooks for 5 channels" << std::endl;
	codebooks.push_back(getCodebook<DisplacementsGetter>(samples));
	codebooks.push_back(getCodebook<HogGetter>(samples));
	codebooks.push_back(getCodebook<HofGetter>(samples));
	codebooks.push_back(getCodebook<MbhXGetter>(samples));
	codebooks.push_back(getCodebook<MbhYGetter>(samples));

	std::cout << "Generating training set" << std::endl;
	generateFeatures(trainingSetNames, trainingSetPath, codebooks, "NoClustering/Features/TrainingSet.out");

	
	/* Similar operation for test set*/
	/*
	std::cout << "Generating test set" << std::endl;
	std::string testSetPath = archivesLocation + "/Test/";
	std::vector<std::string> testSetNames = getArchiveNames(testSetPath);
	generateFeatures(testSetNames, testSetPath, codebooks, "NoClustering/Features/HoG/TestSet.out");
*/
	return 0;
}