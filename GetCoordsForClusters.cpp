#include "BoostRelatedHelpers.h"
// converts sortedTrajectories archive into coords values in a .txt file
// ClusteredTrajectories/r=2/c=500/ 
int main(int argc, char** argv) {
	// path to sortedTrajecotries and result.txt
	std::string graphPath = argv[1];		
	std::string clusterResultPath = argv[2];
	// Reads result.txt, sortedTrajectories.out
	std::unordered_map<int, int> clusterId = readClusterId(clusterResultPath);

	trackList tList;
	restoreTrackList(graphPath + "sortedTrajectories.out", tList);		

	std::ofstream ots(clusterResultPath + "granularUnnormalizedCoords.out");
	{
	  for (const auto& trackIdTrackPair : tList.tracks()) {
	    const track& t = trackIdTrackPair.second;
	    ots << clusterId.find(trackIdTrackPair.first)->second << " " << t.endingFrame << " ";
	    for (size_t i = 0; i < t.coords.size(); ++i) {
	      if ( i != 0) ots << " ";
	      ots << t.coords[i].x << " " << t.coords[i].y;
	    }
	    ots << std::endl;
	  }
	}
	ots.close();
	return 0;
}