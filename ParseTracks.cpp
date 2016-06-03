#include "BoostRelatedHelpers.h"
/* Read *.features, output sortedTrajectories */
// TODO: file and functional documentations
int main(int argc, char** argv) {
	std::string dataset = argv[1];

	std::string videoLocation = argv[2];
	std::string archiveLocation = argv[3];
	
 	std::vector<track> tracks; 
  	std::vector<std::string> trajInStrings;
  	int videoWidth, videoHeight;
  	parseFeaturesToTracks(videoLocation, trajInStrings, tracks, videoWidth, videoHeight); 
  	std::cout << "[ParseTracks] "<< tracks.size() << " trajectories in total" << std::endl;

  	trackList tList;
  	// Stuff dummy trajectory indices
  	for (const auto & t : tracks) {
	    tList.addTrack(0, t);
	}

  	std::string actionCategory = argv[4];

	int vid = std::stoi(argv[5]);

	int actionIndex = -1;
	/* Obviously needs refactoring*/
	if (dataset.compare("OlympicSports") == 0) {
		actionIndex = OlympicActionClassMap[actionCategory];
	} else if (dataset.compare("UCFSports") == 0){
		actionIndex = ucfActionClassMap[actionCategory];
	} else if (dataset.compare("sJHMDB") == 0) {
		actionIndex = subJHMDBActionClassMap[actionCategory];
	} else {
		std::cout << "[ParseTracks] Error, unable to find mapping for dataset " << dataset << std::endl;
	}

	if (actionIndex == -1 || std::isnan(actionIndex) || std::isnan(videoWidth) || std::isnan(vid)) {
		std::cout << "fuck" << std::endl;
		std::cout << actionIndex << std::endl;
	}
	
	videoRep video(tList, actionIndex, vid, videoWidth, videoHeight);
  	std::ofstream ofs(archiveLocation + actionCategory + "_" + std::to_string(vid) + ".out");
	{
	    boost::archive::binary_oarchive oa(ofs);
	    oa << video;   	 							// archive and stream closed when destructors are called
	}
}
