#include "BoostRelatedHelpers.h"
/* Read *.features, output sortedTrajectories */
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

	std::istringstream ss(argv[5]);
	int vid;
	ss >> vid;

	int actionIndex;
	/* Obviously needs refactoring*/
	if (dataset.compare("OlympicSports") == 0) {
			actionIndex = OlympicActionClassMap[actionCategory];
	} else {
			actionIndex = ucfActionClassMap[actionCategory];
	}	
	if (std::isnan(actionIndex) || std::isnan(videoWidth) || std::isnan(vid)) {
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