#include <random>
#include <vector>
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
constexpr float validationPercentage = 0.2;
constexpr int numCenters = 4000;
constexpr int numCodebookIter = 1;

using ClassToFileNames = std::map<std::string, std::vector<std::string>>;
using LabelAndTracks = std::pair<int, std::vector<track>>;

/* Reads and load a bunch of archives (from a specified location) in memory and output feature vectors */
void getArchiveNames(const std::string archivesLocation, std::vector<std::string>& archiveNames) {
	DIR* dirp = opendir(archivesLocation.c_str());
	dirent* dp = NULL;
	while ((dp = readdir(dirp)) != NULL) {
		std::string name = dp->d_name;
		if(name.length() > 2) {
			archiveNames.push_back(name);
		}
	}
	closedir(dirp);
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
	bool isDirty;
	for (const auto& t : tracks) {
		isDirty = false;
		std::for_each(t.coords.begin(), t.coords.end(), [&isDirty](const point& p){
			if (std::isnan(p.x) || std::isnan(p.y))	isDirty = true;
		});

		if(std::any_of(t.hog.begin(), t.hog.end(), [](const float& f){return std::isnan(f);})
		|| std::any_of(t.hof.begin(), t.hof.end(), [](const float& f){return std::isnan(f);})
		|| std::any_of(t.mbhx.begin(), t.mbhx.end(), [](const float& f){return std::isnan(f);})
		|| std::any_of(t.mbhy.begin(), t.mbhy.end(), [](const float& f){return std::isnan(f);})) {
			isDirty = true;
		}
		if (isDirty) std::cout << "===========================Dirty data============================" << std::endl;
	}
}

template <typename Functor>
std::unique_ptr<float[]> getData (const std::vector<track>& tracks) {
	//std::cout << "Filling data array" << std::endl;
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

ClassToFileNames sortFilenamesByClass(const std::vector<std::string>& archiveNames) {
	ClassToFileNames classToFilenames;
	for (const auto& name : archiveNames) {
		size_t pos = name.find("_");
		std::string className = name.substr(0, pos);
		classToFilenames[className].emplace_back(name);
	}
	return classToFilenames;
}

std::vector<std::string> drawValidationSet(ClassToFileNames& copyOfClassToNames) {
	std::vector<std::string> validationSet;
	for (auto& pair : copyOfClassToNames) {
		// For each class, random shuffle the names
		std::random_shuffle(pair.second.begin(), pair.second.end());
		// and then draw 20% as validation set
		int classSize = pair.second.size() * validationPercentage;
		
		// Leave one out strategy
		validationSet.emplace_back(pair.second.back());	
		pair.second.pop_back();		
	}

	return validationSet;
}

std::vector<std::string> gatherTrainingSet(ClassToFileNames& copyOfClassToNames) {
	std::vector<std::string> trainingSet;
	for (auto& pair: copyOfClassToNames) {
		std::move(pair.second.begin(), pair.second.end(), std::back_inserter(trainingSet));
	}
	return trainingSet;
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

std::vector<float> transformData(VlKMeans* kmeans, float* data, int numTracks) {
	//std::unique_ptr<vl_uint32[]> assignments(new vl_uint32[numTracks]);
	//std::unique_ptr<float[]> distances(new float[numTracks]);
	vl_uint32* assignments = (vl_uint32*)vl_malloc(sizeof(vl_uint32) * numTracks);
	float* distances = (float*)vl_malloc(sizeof(float) * numTracks);
	std::vector<float> featureVector(numCenters, 0.0);
	//std::cout << "Transform data" << std::endl;
	vl_kmeans_quantize(kmeans, assignments, distances, data, numTracks);
	for (int i = 0; i < numTracks; ++i) {
		int a = (int32_t)assignments[i];
		if (a >= featureVector.size()) {
			std::cout << "Invalid assignment: " << assignments[i] << " " << a << " from track #" << i << std::endl;
			continue;
		}
		featureVector[a] += static_cast<float>(1.0 / numTracks);	
	}
	return featureVector;		// rvo
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

void generateFeatures(const std::vector<std::string>& filenames, const std::string& archivesLocation, const std::vector<std::unique_ptr<VlKMeans>>& codebooks, const std::string& outputName) {
	// transform data to features
	std::vector<LabelAndTracks> videos = getVideoLabelToTracks(filenames, archivesLocation);
	std::multimap<int, std::vector<float>> outputFile;	// label -> feaure

	for (const auto& instance : videos) {	// Each instance corresponds to a video
		if (instance.second.empty())	continue;
		checkContainNaN(instance.second);
		std::vector<float> feature = transformData(codebooks[0].get(), getData<NormalizedPointGetter>(instance.second).get(), instance.second.size());
		std::vector<float> Hogfeature = transformData(codebooks[1].get(), getData<HogGetter>(instance.second).get(), instance.second.size());
		feature.insert(feature.end(), Hogfeature.begin(), Hogfeature.end());
		
		std::vector<float> Hoffeature = transformData(codebooks[2].get(), getData<HofGetter>(instance.second).get(), instance.second.size());
		feature.insert(feature.end(), Hoffeature.begin(), Hoffeature.end());
		
		std::vector<float> Mbhxfeature = transformData(codebooks[3].get(), getData<MbhXGetter>(instance.second).get(), instance.second.size());
		feature.insert(feature.end(), Mbhxfeature.begin(), Mbhxfeature.end());
		
		std::vector<float> Mbhyfeature = transformData(codebooks[4].get(), getData<MbhYGetter>(instance.second).get(), instance.second.size());
		feature.insert(feature.end(), Mbhyfeature.begin(), Mbhyfeature.end());

		// Write a line to output
		outputFile.emplace(instance.first, feature);
		writeFeaturesToFile(outputName, outputFile);
	}
}

int main(int argc, char** argv) {
	// Get all archive names in the specified folder
	std::string archivesLocation = argv[1];
	archivesLocation += "/Training/";
	std::vector<std::string> archiveNames;
	getArchiveNames(archivesLocation , archiveNames);

	// Randomly sample some trajectories 
	std::vector<track> samples(kNumRandomSamples);

	getTrainingSamples(samples, archiveNames, archivesLocation);

	// Compute codebook from samples
	std::cout << "Compute 5 codebooks" << std::endl;
	std::vector<std::unique_ptr<VlKMeans>> codebooks;
	codebooks.push_back(getCodebook<NormalizedPointGetter>(samples));
	codebooks.push_back(getCodebook<HogGetter>(samples));
	codebooks.push_back(getCodebook<HofGetter>(samples));
	codebooks.push_back(getCodebook<MbhXGetter>(samples));
	codebooks.push_back(getCodebook<MbhYGetter>(samples));

	// Deal with training/test sets
	ClassToFileNames classToFilenames = sortFilenamesByClass(archiveNames);

	std::vector<std::string> trainingSetFilenames = gatherTrainingSet(classToFilenames);
	std::cout << "Generating features" << std::endl;
	/* NaN checks for each file*/
	generateFeatures(trainingSetFilenames, archivesLocation, codebooks, "NoClustering/Features/TrainingSet.out");
	
	}
	return 0;
}