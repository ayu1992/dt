#include "BoostRelatedHelpers.h"

float lengthNormalizedOverlapForTrajectory(const track& t, const std::vector<Box>& boxes) {
	int overlapCount = 0;
	for(int i = 0; i < t.coords.size(); i++) {
		int frame = t.endingFrame + i + 1 - t.coords.size();
		if (isInBox(t.coords[i], boxes[frame])) ++overlapCount;
	}

	return static_cast<float>(overlapCount) / t.length;
}

float countAverageTrajectoriesInBox(const trackList& tList, const std::vector<Box>& boxes, const float threshold) {
	
	int qualifiedTrajectoryCount = 0; 	// number of tracks that have length-normalized overlap score > threshold 
	for (const auto& trackIdPair : tList.tracks()) {
		if (lengthNormalizedOverlapForTrajectory(trackIdPair.second, boxes) > threshold)	qualifiedTrajectoryCount += 1;	
	}

	// return averge number of qualified trajectories
	return static_cast<float>(qualifiedTrajectoryCount) / tList.size();
}

// ClusteredTrajectories/r=0.03/c=2/ InputVideos/ Diving vid thresh 
int main(int argc, char** argv) {
  std::string inpath = argv[1];
  std::string resolution = argv[2];
  std::string category = argv[3];
  std::string bbpath = resolution + category + "/";

  int vid;
  std::istringstream getVid(argv[4]);
  getVid >> vid; 

  videoRep video;
  restoreVideoRep(inpath + "VideoRepresentation.out", video);
   
  std::cout << video.largestCluster().size() << "tracks in total" << std::endl;
  if(video.largestCluster().size() < 50) {
  	std::cout << "ERROR: less than 50 tracks, quit" << std::endl;
  	return -1;
  }

  std::vector<Box> boxes = readBoundingBoxes(bbpath + std::to_string(vid) + ".txt");

  std::ofstream fout;
  fout.open(inpath + "scores_" + category + ".txt", std::ifstream::in | std::ofstream::out | std::ofstream::app);

  static const float arr[] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6};
  std::vector<float> thresholds(arr, arr + sizeof(arr)/sizeof(arr[0]));
  for (const auto& thresh : thresholds) {
	  fout << thresh << ":" << countAverageTrajectoriesInBox(video.largestCluster(), boxes, thresh) << std::endl;  	
  }
  fout.close();
  
return 0;  
}