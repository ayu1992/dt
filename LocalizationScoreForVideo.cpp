#include "ParserHelpers.h"

float lengthNormalizedOverlapForTrajectory(
	const track& t, 
	const std::vector<Box>& boxes,
	const std::unordered_map<int, int>& clusterId,
	const int largestClusterId) {

	std::vector<point> points(t.coords);
	unnormalizePoints(points, t.length, t.mean_x, t.mean_y);	

	int overlapCount = 0;
	for(int i = 0; i < points.size(); i++) {
		point resizedPoint = points[i];
		int frame = t.endingFrame + i + 1 - points.size();
		if (isInBox(resizedPoint, boxes[frame])) ++overlapCount;
	}

	return static_cast<float>(overlapCount) / t.length;
}

float countAverageTrajectoriesInBox(
	const trackList& tList, 
	const std::vector<Box>& boxes, 
	const float threshold,
	const std::unordered_map<int, int>& clusterId,
	const int largestClusterId,
	const int largestClusterSize) {
	
	int qualifiedTrajectoryCount = 0; 	// number of tracks that have length-normalized overlap score > threshold 
	std::cout << largestClusterId << std::endl;
	for (auto const& trackIdPair : tList.tracks()) {
		// Ignore tracks that belong to other clusters
		if (clusterId.find(trackIdPair.first)->second != largestClusterId) {
			continue;
		}
		if (lengthNormalizedOverlapForTrajectory(trackIdPair.second, boxes, clusterId, largestClusterId) > threshold) {
			qualifiedTrajectoryCount += 1;	
		} 
	}

	// return averge number of qualified trajectories
	return static_cast<float>(qualifiedTrajectoryCount) / largestClusterSize;
}
void restoreArchive(const std::string& path, trackList& tList) {
	  std::ifstream ifs(path);		// contains A video
	  boost::archive::binary_iarchive ia(ifs);
	  // restore the schedule from the archive
	  ia >> tList;
}

// ClusteredTrajectories/r=0.03/c=2/ InputVideos/ Diving numClusters vid thresh 
int main(int argc, char** argv) {
  std::string inpath = argv[1];
  std::string resolution = argv[2];
  std::string category = argv[3];
  std::string bbpath = resolution + category + "/";

  int numClusters;
  std::istringstream getNumClusters(argv[4]);
  getNumClusters >> numClusters;

  std::unordered_map<int, int> clusterId;
  std::vector<int> clusterSizes(numClusters, 0);
  int largestClusterId = returnIdOfLargestCluster(inpath + "result.txt", clusterId, clusterSizes);
  int largestClusterSize = clusterSizes[largestClusterId];

  int vid;
  std::istringstream getVid(argv[5]);
  getVid >> vid; 

  trackList tList;
  restoreArchive(inpath + "sortedTrajectories.out", tList);

  std::cout << tList.size() << "tracks in total" << std::endl;
  if(tList.size() < 10) {
  	std::cout << "ERROR: less than 10 tracks, quit" << std::endl;
  	return -1;
  }

  std::vector<Box> boxes = readBoundingBoxes(bbpath + std::to_string(vid) + ".txt");

  std::ofstream fout;
  fout.open(inpath + "scores_" + category + ".txt", std::ifstream::in | std::ofstream::out | std::ofstream::app);

  static const float arr[] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6};
  std::vector<float> thresholds(arr, arr + sizeof(arr)/sizeof(arr[0]));
  for (const auto& thresh : thresholds) {
	  fout << thresh << ":" << countAverageTrajectoriesInBox(tList, boxes, thresh, clusterId, largestClusterId, largestClusterSize) << std::endl;  	
  }
  fout.close();
  
return 0;  
}