#include "CodebookHelpers.h"
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
/* Seperate data into training and testing sets
   Delay cross validation to libsvm
*/
void generateTrainingAndTestSets(
	const std::map<int, std::vector<int>>& usableVideos, 
	std::map<int, std::vector<int>>& vidForTraining, 
	std::map<int, std::vector<int>>& vidForTesting) {

	for (auto& actionClass : usableVideos) {
		int numVideos = actionClass.second.size();	// 40% training, 30% valid, 30% test
		int numTest = numVideos * 0.3;	
		int numTraining = numVideos - numTest;
		// Make a copy and shuffle
		std::vector<int> totalVideos(actionClass.second);
		auto engine = std::default_random_engine{};
		std::random_shuffle(totalVideos.begin(), totalVideos.end());

		std::vector<int> trainingVideos (totalVideos.begin(), totalVideos.begin() + numTraining);
		vidForTraining.emplace(actionClass.first, trainingVideos);

		std::vector<int> testVideos (totalVideos.begin() + numTraining, totalVideos.end());
		vidForTesting.emplace(actionClass.first, testVideos);
	}
}

using RepeatedFloat = google::protobuf::RepeatedField<float>;

struct NormalizedPointGetter {
	const RepeatedFloat& operator()(const motionClustering::Trajectory& t) { return t.normalizedpoints(); }
	static constexpr int dimension = 30;
};

struct HoGGetter {
	const RepeatedFloat& operator()(const motionClustering::Trajectory& t) { return t.hog(); }
	static constexpr int dimension = 96;
};

struct HoFGetter {
	const RepeatedFloat& operator()(const motionClustering::Trajectory& t) { return t.hof(); }
	static constexpr int dimension = 108;
};

struct MbhXGetter {
	const RepeatedFloat& operator()(const motionClustering::Trajectory& t) { return t.mbhx(); }
	static constexpr int dimension = 96;
};

struct MbhYGetter {
	const RepeatedFloat& operator()(const motionClustering::Trajectory& t) { return t.mbhy(); }
	static constexpr int dimension = 96;
};

// cap : const for each class
template <typename Functor>
std::unique_ptr<float[]> getDataForCodebookWithClassCap (int& numData, const std::map<int, std::vector<int>>& vidForTraining, const motionClustering::VideoList& videoList, int classTrajectoryCap) {
	// number of rows in data 
	numData = 0;
	std::map<int, int> classTrajectoryCount;
	for (const auto& actionClass : vidForTraining) {
    	int classTrackSize = 0;
    	// Count total number of trajectories in each class
    	for (int vid : actionClass.second) {
    		classTrackSize += videoList.videos(vid).tracks_size();
    	}	
   			numData += std::min(classTrackSize, classTrajectoryCap);
   			classTrajectoryCount.emplace(actionClass.first, classTrackSize);
    }
    
	int row = 0;		// helps writing into 1D array
	std::unique_ptr<float[]> data(new float[numData * Functor::dimension]);
	for (const auto& actionClass : vidForTraining) {
		if (classTrajectoryCount[actionClass.first] < classTrajectoryCap) {
			// insert all tracks in that class
			for (int vid : actionClass.second) {
				appendTracksToData<Functor>(videoList.videos(vid).tracks(), row, data.get());				
			}
		} else {
			// randomly select some of it
			std::vector<bool> select(classTrajectoryCount[actionClass.first], false);
			
			// Fill the boolean array with ${classTrajectoryCap} 'true's' and false for rest
			select.assign(classTrajectoryCap, true);

			auto select_it = select.begin();
			for (int vid : actionClass.second) {
				// append track if it is selected
				for (auto& t : videoList.videos(vid).tracks()) {
					if (*select_it) {
						for (int i = 0; i < Functor::dimension; ++i) {
							data[row * Functor::dimension + i] = Functor()(t).Get(i);	//t.normalizedpoints(i);
						}
						++row;
					}
					++select_it;					
				}
			}
		}
	}

	return data;
}

template <typename Functor>
void generateTrainingAndTestFeaturesForChannel(
	std::vector<std::vector<float>>& trainingFeaturesForChannel, 		// num of videos x 4000
	std::vector<std::vector<float>>& testFeaturesForChannel,
	const std::map<int, std::vector<int>>& vidForTraining,
	const std::map<int, std::vector<int>>& vidForTesting,
	const motionClustering::VideoList& videoList) {

	// Training + Vali -> Codebook
    int numData = 0;
    std::unique_ptr<float[]> data = getDataForCodebookWithClassCap<Functor>(numData, vidForTraining, videoList, classTrajectoryCap);

	std::vector<std::unique_ptr<VlKMeans>> kmeansInstances;
	std::vector<float *> kmeansCenters;							// TODO : change to unique ptrs ?
  	std::vector<float> sumDistances;

	int codebook_index = makeCodebook<Functor>(data.get(), numData, kmeansInstances, kmeansCenters, sumDistances);	
	VlKMeans* codebookKmeans = kmeansInstances[codebook_index].get();

	std::cout << "[Codebook] Generate training and test set" << std::endl;
    numData = 0;
	int row = 0;
    // Each class generates some training datas
	for (const auto& actionClass : vidForTraining) {
		
		// One write per video
		for (int vid : actionClass.second) {
			numData = videoList.videos(vid).tracks_size();
			std::unique_ptr<float[]> trainingData(new float[numData * Functor::dimension]);		

			// Fill up trainingData 
			row = 0;
			appendTracksToData<Functor>(videoList.videos(vid).tracks(), row, trainingData.get());
			
			trainingFeaturesForChannel.push_back(transformData(codebookKmeans, trainingData.get(), numData));
		}
	}

	/** GENERATE TEST SETS **/
	int numTestData = 0;
	for (const auto& actionClass : vidForTesting) {
		for (int vid : actionClass.second) {
			numTestData = videoList.videos(vid).tracks_size();
			std::unique_ptr<float[]> testingData(new float[numTestData * Functor::dimension]);
			
			row = 0;
			appendTracksToData<Functor>(videoList.videos(vid).tracks(), row, testingData.get());
			
			testFeaturesForChannel.push_back(transformData(codebookKmeans, testingData.get(), numTestData));			
		}
	}
}

int main(int argc, char* argv[]) {

	srand(0);

	motionClustering::VideoList videoList;
	
	std::string filepath = argv[1];
	std::string dumpPath = filepath + "TrajectoryDump.data";

	int input = open(dumpPath.c_str(), O_RDONLY);

	if (!input) {
	    std::cout << ": File not found.  Creating a new file later." << std::endl;
	} else {
	  google::protobuf::io::ZeroCopyInputStream* infile = new google::protobuf::io::FileInputStream(input);
	  google::protobuf::io::CodedInputStream* coded_input = new google::protobuf::io::CodedInputStream(infile);
	  coded_input->SetTotalBytesLimit(500 << 20, 300 << 20);
	  if (!videoList.ParseFromCodedStream(coded_input)) {
	    std::cerr << "Failed to parse videos QQ" << std::endl;
	    return -1;
	  }
	}

	std::cout << "Total data dump contains " << countTracks(videoList) << " trajectories" << std::endl;

	// action label -> indices in videoList (videos with > 0 tracks only)
	std::map<int, std::vector<int>> usableVideos = parseVideoList(videoList);

    // Partition data into training set and testing set
    // class id -> vids'
    std::map<int, std::vector<int>> vidForTraining;
    std::map<int, std::vector<int>> vidForTesting;

    generateTrainingAndTestSets(usableVideos, vidForTraining, vidForTesting);

	/* Repeat this for every channel */
	
	std::vector<std::vector<float>> trainingFeaturesForPoints;
	std::vector<std::vector<float>> testingFeaturesForPoints;
	generateTrainingAndTestFeaturesForChannel<NormalizedPointGetter>(trainingFeaturesForPoints, testingFeaturesForPoints, vidForTraining, vidForTesting, videoList);	
	
	
	std::vector<std::vector<float>> trainingFeaturesForHoG;
	std::vector<std::vector<float>> testingFeaturesForHoG;
	generateTrainingAndTestFeaturesForChannel<HoGGetter>(trainingFeaturesForHoG, testingFeaturesForHoG, vidForTraining, vidForTesting, videoList);
	

	std::vector<std::vector<float>> trainingFeaturesForHoF;
	std::vector<std::vector<float>> testingFeaturesForHoF;
	generateTrainingAndTestFeaturesForChannel<HoGGetter>(trainingFeaturesForHoF, testingFeaturesForHoF, vidForTraining, vidForTesting, videoList);

	std::vector<std::vector<float>> trainingFeaturesForMbhX;
	std::vector<std::vector<float>> testingFeaturesForMbhX;
	generateTrainingAndTestFeaturesForChannel<HoGGetter>(trainingFeaturesForMbhX, testingFeaturesForMbhX, vidForTraining, vidForTesting, videoList);

	std::vector<std::vector<float>> trainingFeaturesForMbhY;
	std::vector<std::vector<float>> testingFeaturesForMbhY;
	generateTrainingAndTestFeaturesForChannel<HoGGetter>(trainingFeaturesForMbhY, testingFeaturesForMbhY, vidForTraining, vidForTesting, videoList);	

	std::cout << trainingFeaturesForPoints.size() << std::endl;
	std::cout << trainingFeaturesForHoG.size() << std::endl;
	std::cout << trainingFeaturesForHoF.size() << std::endl;
	std::cout << trainingFeaturesForMbhX.size() << std::endl;
	std::cout << trainingFeaturesForMbhY.size() << std::endl;
	
	/*
	std::cout << "Writing training features to file" << std::endl;
	writeFeaturesToFile(filepath + "TrainingSet.data", videoList, vidForTraining, trainingFeaturesForPoints, trainingFeaturesForHoG, trainingFeaturesForHoF, trainingFeaturesForMbhX, trainingFeaturesForMbhY);
	//writeSingleChannelToFile(filepath + "TrainingSet.data", videoList, vidForTraining, trainingFeaturesForMbhX);
	std::cout << "Wrinting test set features to file" << std::endl;
	writeFeaturesToFile(filepath + "TestingSet.data", videoList, vidForTesting, testingFeaturesForPoints, testingFeaturesForHoG, testingFeaturesForHoF, testingFeaturesForMbhX, testingFeaturesForMbhY);
	//writeSingleChannelToFile(filepath + "TestingSet.data", videoList, vidForTesting, testingFeaturesForMbhX);
	*/
	
	
	std::array<float, 5> Ac = writeTrainingKernelsToFile(
		filepath + "TrainingKernel.data", 
		videoList, 
		vidForTraining,
		trainingFeaturesForPoints, 
		trainingFeaturesForHoG, 
		trainingFeaturesForHoF, 
		trainingFeaturesForMbhX, 
		trainingFeaturesForMbhY);

	writeTestingKernelsToFile(
		filepath + "TestingKernel.data",
		videoList,
		vidForTesting,
		Ac,
		testingFeaturesForPoints,
		trainingFeaturesForPoints,
		testingFeaturesForHoG,
		trainingFeaturesForHoG,
		testingFeaturesForHoF,
		trainingFeaturesForHoF,
		testingFeaturesForMbhX,
		trainingFeaturesForMbhX,
		testingFeaturesForMbhY,
		trainingFeaturesForMbhY);
	
  	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}