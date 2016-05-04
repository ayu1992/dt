#include "BoostRelatedHelpers.h"
/* Filters out irrelevnat clusters
 * Adds action label
**/
int main(int argc, char** argv) {
	// path to results.txt, sortedTrajectories.out
	std::string path = argv[1];
	std::string actionCategory = argv[2];

	std::istringstream ss(argv[3]);
	int vid;
	ss >> vid;

	std::unordered_map<int, int> clusterId;
    int largestClusterId = returnIdOfLargestCluster(path + "result.txt", clusterId);

    trackList tList;
	restoreTrackList(path + "sortedTrajectories.out", tList);	
	std::cout << tList.size() << "tracks in total" << std::endl;

	trackList filteredTList;
	for (const auto& trackIdTrackPair : tList.tracks()) {
		int trackId = trackIdTrackPair.first;
		// Ignore tracks that belong to other clusters
		if (clusterId.find(trackId)->second != largestClusterId) {
			continue;
		}
		filteredTList.addTrack(trackId, trackIdTrackPair.second);
	}

	videoRep video(filteredTList, actionClassMap[actionCategory], vid);

	/*TODO: make this append*/
    std::ofstream ofs(path + "VideoRepresentation.out");
	{
	    boost::archive::binary_oarchive oa(ofs);
	    oa << video;    // archive and stream closed when destructors are called
	}

	// Generate text output for DrawCluster (requires to be linked with opencv)
	// endingFrame, coords
	writeCoordsToFile(path, filteredTList);
	return 0;
}