#include <iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include "dump.pb.h"
#include <fcntl.h>
#include <unistd.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
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
			auto val_from = val.begin() + 6;
			auto val_to = val_from + TRACK_LEN * 2;
			std::copy(val_from, val_to, google::protobuf::RepeatedFieldBackInserter(newTrajectory->mutable_normalizedpoints()));
			val_from = val_to;
			val_to += HOG_DIM;
			std::copy(val_from, val_to, google::protobuf::RepeatedFieldBackInserter(newTrajectory->mutable_hog()));
			val_from = val_to;
			val_to += HOF_DIM;
			std::copy(val_from, val_to, google::protobuf::RepeatedFieldBackInserter(newTrajectory->mutable_hof()));
			val_from = val_to;
			val_to += MBHX_DIM;
			std::copy(val_from, val_to, google::protobuf::RepeatedFieldBackInserter(newTrajectory->mutable_mbhx()));
			val_from = val_to;
			val_to += MBHY_DIM;
			std::copy(val_from, val_to, google::protobuf::RepeatedFieldBackInserter(newTrajectory->mutable_mbhy()));
		}

		numLines++;
	}
	std::cout << "There are a total of " << numLines << "tracks after pspectral" << std::endl;
}

int main(int argc, char** argv) { // argv format: output path, actionLabel, video index, num clusters used during pspectral
	// trj -> cid
	std::unordered_map<int, int> clusterId;

	std::string outpath = argv[1];
	// trajectory id -> vector<Point2f>, scales, frames
	const std::string trjFilename = outpath + "sortedTrajectories.out";			// contains A video

	// Read result.txt	
	readClusterId(outpath + "result.txt", clusterId);
	std::cout << "cid size : " << clusterId.size() << std::endl;

	outpath += "TrajectoryDump.data";
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

	motionClustering::VideoList videos;

	int input = open(outpath.c_str(), O_RDONLY);

	if (!input) {
	    std::cout << ": File not found.  Creating a new file later." << std::endl;
	} else {
		google::protobuf::io::ZeroCopyInputStream* infile = new google::protobuf::io::FileInputStream(input);
		google::protobuf::io::CodedInputStream* coded_input = new google::protobuf::io::CodedInputStream(infile);
		coded_input->SetTotalBytesLimit(400 << 20, 200 << 20);
		if (!videos.ParseFromCodedStream(coded_input)) {
			std::cerr << "Failed to parse videos QQ" << std::endl;
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