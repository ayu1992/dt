#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include "dump.pb.h"

/* Make this into a script that prunes 0 trajectories*/
int main(int argc, char** argv) {
  
  motionClustering::VideoList videoList;
  int input = open(argv[1], O_RDONLY);

  if (!input) {
      std::cout << ": File not found.  Creating a new file later." << std::endl;
  } else {
    google::protobuf::io::ZeroCopyInputStream* infile = new google::protobuf::io::FileInputStream(input);
    google::protobuf::io::CodedInputStream* coded_input = new google::protobuf::io::CodedInputStream(infile);
    coded_input->SetTotalBytesLimit(600 << 20, 200 << 20);
    if (!videoList.ParseFromCodedStream(coded_input)) {
      std::cerr << "Failed to parse videos QQ" << std::endl;
      return -1;
    }
  }

  	for(int i = 0; i < videoList.videos_size(); i++) {
  		std::cout << "Video : " << i << std::endl;
  		const motionClustering::VideoInstance&  video = videoList.videos(i);
  		std::cout << "Action Category : " << video.actionlabel() << std::endl;
  		std::cout << "Video index : " << video.videoindex() << std::endl;
  		std::cout << "Num Tracks :" << video.tracks_size() << std::endl;

      if (video.tracks_size() > 0) {
        std::cout << video.tracks(0).normalizedpoints_size() << std::endl;        
        std::cout << video.tracks(0).hog_size() << std::endl;
        std::cout << video.tracks(0).hof_size() << std::endl;
        std::cout << video.tracks(0).mbhx_size() << std::endl;
        std::cout << video.tracks(0).mbhy_size() << std::endl;
      }
      /*
      for (auto& t : video.tracks()) {
        std::cout << t.hog_size() << std::endl;
        std::cout << t.normalizedpoints_size() << std::endl;
        for (float hog : t.hog()) {
          std::cout << hog << ",";
        }
        std::cout << std::endl;
      }*/

  	}
  	
  	return 0;
}