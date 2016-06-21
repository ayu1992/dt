#include "BoostRelatedHelpers.h"

trackList extractTracksInLargestCluster(
	const trackList& tList, 
	const std::unordered_map<int, int>& clusterId, 
	const int maxNumClusters) {
	trackList largestCluster;

	int largestClusterId;
	// Compute cluster sizes
	std::vector<int> clusterSizes(maxNumClusters, 0);
	for (const auto& tidCidPair : clusterId) {
		clusterSizes[tidCidPair.second] += 1;
	}

	int largestClusterIdx = std::max_element(clusterSizes.begin(), clusterSizes.end()) - clusterSizes.begin();

	// test
	for (const auto& tidCidPair : clusterId) {
		if (tidCidPair.second == largestClusterIdx)
			largestCluster.addTrack(-1, tList.getTrack(tidCidPair.first));	// original tid doesn't matter any more
	}

	std::cout << "Largest cluster contains " << largestCluster.size() << " tracks " << std::endl;
	return largestCluster;
}

int main(int argc, char** argv) {
	const std::string primitiveGraphPath = argv[1];
	const std::string clusterResultPath = argv[2];	
	const int maxNumClusters = std::stoi(argv[3]);
	const std::string videoCategory = argv[4];
	const std::string vid = argv[5];

	// Reads result.txt, sortedTrajectories.out
	// track id -> cid
	std::cout << "reading cluster assignments" << std::endl;
	std::unordered_map<int, int> clusterId = readClusterId(clusterResultPath);

	const std::string videoName = videoCategory + vid;
	trackList tList;
	restoreTrackList(primitiveGraphPath + videoName + "_sortedTrajectories.out", tList);	
	std::cout << tList.tracks().size() << " tracks are restored" << std::endl;

	trackList largestCluster = extractTracksInLargestCluster(tList, clusterId, maxNumClusters);

	
	std::cout << "Output tracks from largest cluster to archive" << std::endl;
	videoRep video(largestCluster, ucfActionClassMap[videoCategory], std::stoi(vid), -1, -1);	// Set videoWidth and videoHeight to nil
  	std::ofstream ofs(clusterResultPath + "largestCluster/" +videoCategory + "_" + vid + ".out");
	{
	    boost::archive::binary_oarchive oa(ofs);
	    oa << video;   	 							// archive and stream closed when destructors are called
	}

	return 0;
}