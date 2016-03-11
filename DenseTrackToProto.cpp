#include "Util.h"
#include "ParserHelpers.h"
#include "dump.pb.h"
#include <algorithm>
#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

// Read Dense tracks for a video
// Stuff it in protobuf
void parseStringsToTrajectories(const std::vector<std::string>& trajInStrings, motionClustering::VideoInstance* this_video) {
	int line = 0;
	for (auto const& str: trajInStrings) {
    	std::vector<float> val = split(str, '\t');
		motionClustering::Trajectory* newTrajectory = this_video->add_tracks();
	    // First 10 elems are track info		
		auto val_from = val.begin() + TRACK_INFO_LEN;
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
	++line;
	}
	std::cout << line << "tracks from DenseTrack" << std::endl;
}

int main(int argc, char** argv) {
  
  std::string outpath = argv[1];
  std::string inpath = argv[2];
  int vid;
  std::istringstream getVid(argv[3]);
  getVid >> vid; 

  float threshold;
  std::istringstream getThreshold(argv[4]);
  getThreshold >> threshold;

  std::vector<std::string> trajInStrings;

  std::cout << "Reading trajectories" << std::endl;
  readFileIntoStrings(inpath + std::to_string(vid) + ".features", trajInStrings);
  
  motionClustering::VideoList videos;

  // Read the proto
  int input = open(outpath.c_str(), O_RDONLY);

  if (!input) {
      std::cout << ": File not found.  Creating a new file later." << std::endl;
  } else {
  	google::protobuf::io::ZeroCopyInputStream* infile = new google::protobuf::io::FileInputStream(input);
 	google::protobuf::io::CodedInputStream* coded_input = new google::protobuf::io::CodedInputStream(infile);
 	coded_input->SetTotalBytesLimit(500 << 20, 200 << 20);
	if (!videos.ParseFromCodedStream(coded_input)) {
		std::cerr << "Failed to parse videos QQ" << std::endl;
		return -1;
	}
  }

  motionClustering::VideoInstance* this_video = videos.add_videos();

  this_video->set_actionlabel(argv[2]);
//  this_video->set_videoindex(videoIndex);
//  this_video->set_numclusters(numClusters);

  parseStringsToTrajectories(trajInStrings, this_video); 
  std::cout << this_video->tracks_size() << " trajectories written to proto" << std::endl;

  // Dump the proto
  std::fstream output( outpath, std::ios::out | std::ios::trunc | std::ios::binary);
  if (!videos.SerializeToOstream(&output)) {
     std::cerr << "Failed to write data dump." << std::endl;
     return -1;
  }

  	google::protobuf::ShutdownProtobufLibrary();

	return 0;  
}