#include <iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include "dump.pb.h"
#include "ParserHelpers.h"

/**
 * Takes result.txt (from pspectralclustering), sortedTrajectories.txt as input
 * Extract the biggest cluster of trajectories, output them in proto
 */

void extractTrajectoriesOfLargestCluster(const std::string& filename, const std::unordered_map<int, int>& clusterId, int largestClusterId, motionClustering::VideoInstance* video) {
	// read the trajectories by line
	std::string line;
	int numLines = 0;
	std::ifstream fin(filename.c_str());
	if (!fin) {
		std::cerr << "Unable to open file : " << filename << std::endl;
		return;
	}

	while(std::getline(fin, line)) {
		std::vector<float> val = split(line, ' ');

		// If this trajectory belongs to the largest cluster, dump it!
		if (largestClusterId == clusterId.find(static_cast<int>(val[0]))->second) {
			motionClustering::Trajectory* newTrajectory = video->add_tracks();
			/*
			This code is fine as is. If you're interested in std::copy, you can do this instead:
			#include <google/protobuf/repeated_field.h>
			...
			std::copy(val.begin() + 6, val.end(),
			          google::protobuf::RepeatedFieldBackInserter(newTrajectory->mutable_normalized_points()));
			*/
			for(auto val_it = val.begin() + 6; val_it != val.end(); ++val_it) {		// refactor with std::copy?
				newTrajectory->add_normalizedpoints(*val_it);
			}
		}
		numLines++;
	}
	std::cout << "There are a total of " << numLines << "tracks after pspectral" << std::endl;
}

int main(int argc, char** argv) { // argv format: output path, actionLabel, video index, num clusters used during pspectral
	// trj -> cid
	std::unordered_map<int, int> clusterId;

	// trajectory id -> vector<Point2f>, scales, frames
	const std::string trjFilename = "sortedTrajectories.txt";			// contains A video

	std::string outpath = argv[1];
	outpath += "TrajectoryDump.data";

	// Read result.txt	
	readClusterId("result.txt", clusterId);

	// TODO: How about std::atoi instead of istringstream?
	// Parse and get video information
	int videoIndex, numClusters;
	std::istringstream getVideoIndex(argv[3]);
  getVideoIndex >> videoIndex;
  std::istringstream getNumClusters(argv[4]);
  getNumClusters >> numClusters;

	// Identify the biggest cluster (cid)	
	std::vector<int> clusterSizes(numClusters, 0);

	for (const auto& pair: clusterId) {
		clusterSizes[pair.second] += 1;
	}

	// TODO: Maybe change arg type of the lambda to "const std::pair<int, size_t>&".
	auto maxElemIter = std::max_element(clusterSizes.begin(), clusterSizes.end());
	int largestClusterId = std::distance(clusterSizes.begin(), maxElemIter);

	std::cout << "Largest cluster id: " << largestClusterId << std::endl;
	// Construct proto information : action label, video id (UCF ordering), number of clusters run on pspectral, number of trajs, 
	// if this video contains more than 0 trajectories, extract largest cluster and write
	// else skip this video

	motionClustering::VideoList videos;
	{
	  // Read the existing trajectory dump
	  std::fstream input( outpath, std::ios::in | std::ios::binary);
	  if (!input) {
	    std::cout << ": File not found.  Creating a new file." << std::endl;
	  } else if (!videos.ParseFromIstream(&input)) {
	    std::cerr << "Failed to parse videos." << std::endl;
	    return -1;
	  }
  }


	motionClustering::VideoInstance* this_video = videos.add_videos();

	this_video->set_actionlabel(argv[2]);
	this_video->set_videoindex(videoIndex);
	this_video->set_numclusters(numClusters);

	// Read sortedTrajectories.txt
	// If the traj belongs to cid, add to proto
  extractTrajectoriesOfLargestCluster(trjFilename, clusterId, largestClusterId, this_video);
  std::cout << "The dominant cluster contains " << clusterSizes[largestClusterId] << " tracks" << std::endl;
  
  std::cout << "So " << this_video->tracks_size() << " tracks will be written to output" << std::endl;
	// Dump the proto
	std::fstream output( outpath, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!videos.SerializeToOstream(&output)) {
	   std::cerr << "Failed to write data dump." << std::endl;
	   return -1;
	}

  google::protobuf::ShutdownProtobufLibrary();
	return 0;
}