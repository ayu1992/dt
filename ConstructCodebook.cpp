#include "CodebookHelpers.h"

int main(int argc, char* argv[]) {

	srand(0);

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
	std::map<int, std::vector<int>> usableVideos = parseVideoList(videoList);

 	printUsableVideos(usableVideos);

    // Partition data into training set, validation set and testing set
    std::map<int, DataSplit> dataSplitForClass;   	// action label -> DataSplit

    generateCrossValidationSets(usableVideos, dataSplitForClass);
    countTrackSizes(dataSplitForClass, videoList);

    int numData = 0;

    std::unique_ptr<float[]> data = getDataForCodebookWithCap(dataSplitForClass, videoList, numData, classTrajectoryCap);

	std::vector<std::unique_ptr<VlKMeans>> kmeansInstances;
	std::vector<float *> kmeansCenters;							// TODO : change to unique ptrs ?
  	std::vector<float> sumDistances;

	int codebook_index = makeCodebook(data.get(), numData, kmeansInstances, kmeansCenters, sumDistances);	
	VlKMeans* codebookKmeans = kmeansInstances[codebook_index].get();


	/** GENERATE TEST SETS **/
	int numTestData = 0;
	std::vector<int> vidInSequence;
	for (const auto& actionClass : dataSplitForClass) {
		for (int vid : actionClass.second.TestingSetVid) {
			numTestData += videoList.videos(vid).tracks_size();
			vidInSequence.push_back(vid);							// can a vector push back a set?
		}
	}
	std::unique_ptr<float[]> testingData(new float[numTestData * dimension]);
	
	int row = 0;
	for (int id : vidInSequence) {
		appendTracksToData(videoList.videos(id).tracks(), row, testingData.get());
	}
	std::unique_ptr<float[]> testFeatures = partitionData(vidInSequence, videoList, codebookKmeans, testingData.get(), numTestData);
	std::cout << "Wrinting test set features to file" << std::endl;
	std::string testingSetPath = filepath + "TestingSet.data";
	writeFeaturesToFile(testingSetPath, testFeatures.get(), videoList, vidInSequence);

	/** GENERATE TRAINING AND VALIDATION SETS **/
    for (int fold = 0; fold < numFolds; ++fold) {
    	/** GENERATE TRAINING SETS **/
     	numData = 0;
		for (const auto& actionClass : dataSplitForClass) {
			numData += actionClass.second.TrainingSetSizes[fold];
		}

		std::unique_ptr<float[]> trainingData(new float[numData * dimension]);
		// Fill up trainingData
		row = 0;
		std::vector<int> trainingVidInSequence;
		for (const auto& actionClass : dataSplitForClass) {
			for (int vid : actionClass.second.TrainingSetVid[fold]) {
				trainingVidInSequence.push_back(vid);
				appendTracksToData(videoList.videos(vid).tracks(), row, trainingData.get());
			}
		}

       	std::unique_ptr<float[]> trainingFeatures = partitionData(trainingVidInSequence, videoList, codebookKmeans, trainingData.get(), numData);
		std::cout << "Writing training set features to file" << std::endl;
	    std::string trainingSetPath = filepath + "TrainingSet" + std::to_string(fold) + ".data";
	    writeFeaturesToFile(trainingSetPath, trainingFeatures.get(), videoList, trainingVidInSequence);

	    /** GENERATE VALIDATION SETS **/
     	numData = 0;
		for (const auto& actionClass : dataSplitForClass) {
			numData += actionClass.second.ValidationSetSizes[fold];
		}

		std::unique_ptr<float[]> validationData(new float[numData * dimension]);
		// Fill up validationData
		row = 0;
		std::vector<int> validationVidInSequence;
		for (const auto& actionClass : dataSplitForClass) {
			for (int vid : actionClass.second.ValidationSetVid[fold]) {
				validationVidInSequence.push_back(vid);
				appendTracksToData(videoList.videos(vid).tracks(), row, validationData.get());
			}
		}

       	std::unique_ptr<float[]> validationFeatures = partitionData(validationVidInSequence, videoList, codebookKmeans, validationData.get(), numData);
		std::cout << "Writing validation set features to file" << std::endl;
	    std::string validationSetPath = filepath + "ValidationSet" + std::to_string(fold) + ".data";
	    writeFeaturesToFile(validationSetPath, validationFeatures.get(), videoList, validationVidInSequence);
     }

  	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}