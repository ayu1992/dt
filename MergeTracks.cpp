#include <functional>
#include <algorithm>

#include "BoostRelatedHelpers.h"
const int MAX_TEMPORAL_DISTANCE = 5;

template <typename T>
void vectorAdd(const std::vector<T>& v1, std::vector<T>& v2) {
	std::transform(v1.begin(), v1.end(), v2.begin(), v2.begin(), std::plus<T>());
}

void addToTrack(const track& newTrack, track& targetTrack) {	// add newTrack to targetTracks
	vectorAdd(newTrack.displacements, targetTrack.displacements);
	vectorAdd(newTrack.coords, targetTrack.coords);
	vectorAdd(newTrack.hog, targetTrack.hog);
	vectorAdd(newTrack.hof, targetTrack.hof);
	vectorAdd(newTrack.mbhx, targetTrack.mbhx);
	vectorAdd(newTrack.mbhy, targetTrack.mbhy);
}

struct TakeAverageForFeature {
  const float numTracksAdded;
  TakeAverageForFeature(int numTracksAdded) : numTracksAdded(numTracksAdded) {}

  template <typename T>
  void operator()(std::vector<T>& v) const {
  	std::transform(v.begin(), v.end(), v.begin(), [this](const T& e){ return e / numTracksAdded; });
  }
};

// find supertrack representation for a group of tracks
track computeSuperTrackFromGroup(const std::vector<track>& tracks, int& qualifyCount) {

	track superTrack(tracks[0]);	// tracks remains sorted by endingFrames
	int numTracksAdded = 0;
	for (const auto& t : tracks) {
		if (t.endingFrame - superTrack.endingFrame <= MAX_TEMPORAL_DISTANCE) {
			addToTrack(t, superTrack);
			++numTracksAdded;
		}
	}

	float utilization =  (float )numTracksAdded / tracks.size();
	if (utilization > 0.5) ++qualifyCount;

	// take average and statistics
	
	if (numTracksAdded) {
		auto take_average_for_feature = TakeAverageForFeature(numTracksAdded);
		take_average_for_feature(superTrack.displacements);
		take_average_for_feature(superTrack.coords);
		take_average_for_feature(superTrack.hog);
		take_average_for_feature(superTrack.hof);
		take_average_for_feature(superTrack.mbhx);
		take_average_for_feature(superTrack.mbhy);
	}

	// recalculate means, var if neccessary

	return superTrack;
}

// clusterId : trajId -> cid
// tList is sorted in endingFrame by ascending order
trackList computeSuperTracks(const std::unordered_map<int, int>& clusterId, const trackList& tList) {
	trackList superTracks;			// rvo
	// place the trajectories to their respective groups
	std::vector<std::vector<track>> groups(500, std::vector<track>());	// there should be 500 clusters, supposedly, Raise error otherwise	
	for (const auto& idTrackPair : tList.tracks()) {	// pair<track id, track>
		int cid = clusterId.find(idTrackPair.first)->second;	
		groups[cid].push_back(idTrackPair.second);	// cid - 1?
	}

	// For each group of trajectories, compute a representation
	int qualifyCount = 0;
	for (const auto& group : groups) {
		if (group.size()) {
			superTracks.addTrack(-1, computeSuperTrackFromGroup(group, qualifyCount));	// -1 as dummy index			
		}
	}
	
	std::cout << (float) qualifyCount * 100 / groups.size() << "% groups used more than half of their tracks" << std::endl;
 	return superTracks;
}

int main(int argc, char** argv) {
	// path to sortedTrajecotries and result.txt
	std::string inpath = argv[1];		
	// Reads result.txt, sortedTrajectories.out
	std::unordered_map<int, int> clusterId = readClusterId(inpath);

	trackList tList;
	restoreTrackList(inpath + "sortedTrajectories.out", tList);	
	std::cout << tList.size() << "tracks in total" << std::endl;

	// Merge trajectories in the same cluster	
	trackList superTracks =	computeSuperTracks(clusterId, tList);								
	// Define and Calc edge weights
	// Output to archive, print edge weights
	writeCoordsToFile(inpath, superTracks);
	return 0;
}