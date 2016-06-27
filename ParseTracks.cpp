/**
 * Reads *.features, parse them into track objects (defined in BoostRelatedHelpers.h)
 * output sorted trajectories.
 */ 
#include "BoostRelatedHelpers.h"

int main(int argc, char** argv) {
	const std::string dataset = argv[1];
	const std::string videoLocation = argv[2];
	const std::string archiveLocation = argv[3];
	
 	std::vector<track> tracks; 
 	// a temporary object to hold the strings read from file
  	std::vector<std::string> trajInStrings;
  	int videoWidth, videoHeight;
  	parseFeaturesToTracks(videoLocation, trajInStrings, tracks, videoWidth, videoHeight); 
  	std::cout << "[ParseTracks] "<< tracks.size() << " trajectories in total" << std::endl;

  	trackList tList;
  	// Stuff dummy trajectory indices, real functional indices will be assigned in BuildGraph.cpp
  	for (const auto & t : tracks) {
	    tList.addTrack(0, t);
	}

  	const std::string actionCategory = argv[4];

	const int vid = std::stoi(argv[5]);

	int actionIndex = -1;

	// Assigning an integer class label for this video.
	// Finds the corresponding label mapping for this dataset (defined in ParserHelper.h)
	if (dataset.compare("OlympicSports") == 0) {
		actionIndex = OlympicActionClassMap[actionCategory];
	} else if (dataset.compare("UCFSports") == 0){
		actionIndex = ucfActionClassMap[actionCategory];
	} else if (dataset.compare("sJHMDB") == 0) {
		actionIndex = subJHMDBActionClassMap[actionCategory];
	} else if (dataset.compare("kth") == 0 ||  dataset.compare("msrii") == 0) {
		actionIndex = kthActionClassMap[actionCategory];
	} else {
		std::cout << "[ParseTracks] Error, unable to find mapping for dataset " << dataset << std::endl;
	}

	if (actionIndex == -1 || std::isnan(actionIndex) || std::isnan(videoWidth) || std::isnan(vid)) {
		std::cout << "fuck" << std::endl;
		std::cout << actionIndex << std::endl;
	}
	
	// Packs everything into a video representation and output
	videoRep video(tList, actionIndex, vid, videoWidth, videoHeight);
  	std::ofstream ofs(archiveLocation + actionCategory + "_" + std::to_string(vid) + ".out");
	{
	    boost::archive::binary_oarchive oa(ofs);
	    oa << video;   	
	}
	return 0;
}
