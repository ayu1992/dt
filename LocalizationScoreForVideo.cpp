#include "Util.h"
#include "ParserHelpers.h"
#include "DenseTrack.h"
#include "CVUtils.h"

float lengthNormalizedOverlapForTrajectory(
	const std::string& track, 
	const std::vector<Box>& boxes,
	const std::unordered_map<int, int>& clusterId,
	const int largestClusterId) {
	
	int trjId, endingFrame;
	float scale, trajectory_length, mean_x, mean_y; 
	std::vector<Point2f> points;
	parseLineFromSortedTrajectories(track, trjId, endingFrame, scale, trajectory_length, mean_x, mean_y, points);
	
	if (clusterId.find(trjId)->second != largestClusterId) {
		return 0.0;
	}

	unnormalizePoints(points, trajectory_length, mean_x, mean_y);	

	int overlapCount = 0;
	for(int i = 0; i < points.size(); i++) {
		Point2f resizedPoint = points[i];
		int frame = endingFrame + i + 1 - points.size();
//		std::cout << " boxes has " << boxes.size() << " frames, accessing frame " << frame << std::endl;
//		std::cout << "box Point : " << boxes[frame].UpperLeft.x << "," << boxes[frame].UpperLeft.y << " width : " << boxes[frame].width << std::endl;
//		std::cout << "Resized point : " << resizedPoint.x << "," << resizedPoint.y << std::endl;
		if (isInBox(resizedPoint, boxes[frame])) ++overlapCount;
	}

	return overlapCount / trajectory_length;
}

float countAverageTrajectoriesInBox(
	const std::vector<std::string>& trajInStrings, 
	const std::vector<Box>& boxes, 
	const float threshold,
	const std::unordered_map<int, int>& clusterId,
	const int largestClusterId,
	const int largestClusterSize) {
	
	int qualifiedTrajectoryCount = 0; 	// number of tracks that have length-normalized overlap score > threshold 
	std::cout << trajInStrings.size() << "tracks in total" << std::endl;
//	int cnt = 0;
	for (auto const& track : trajInStrings) {
//		std::cout << "processing track " << cnt << std::endl;
		if (lengthNormalizedOverlapForTrajectory(track, boxes, clusterId, largestClusterId) > threshold) {
			qualifiedTrajectoryCount += 1;	
		} 
//		cnt++;
	}

	// return averge number of qualified trajectories
	return static_cast<float>(qualifiedTrajectoryCount) / largestClusterSize;
}

// Reads sortedTrajectories, output localization score of the largest cluster
// ClusteredTrajectories/r=0.03/c=2/ InplutVideos/Diving/ numClusters vid thresh 
int main(int argc, char** argv) {
  std::string inpath = argv[1];
  std::string category = argv[2];
  std::string bbpath = "ori/" + category + "/";

  int numClusters;
  std::istringstream getNumClusters(argv[3]);
  getNumClusters >> numClusters;

  std::unordered_map<int, int> clusterId;
  std::vector<int> clusterSizes(numClusters, 0);
  int largestClusterId = returnIdOfLargestCluster(inpath + "result.txt", clusterId, clusterSizes);
  int largestClusterSize = clusterSizes[largestClusterId];

  int vid;
  std::istringstream getVid(argv[4]);
  getVid >> vid; 

  float threshold;
  std::istringstream getThreshold(argv[5]);
  getThreshold >> threshold;

  std::vector<std::string> trajInStrings;

  const std::string trjFilename = inpath + "sortedTrajectories.out";			// contains A video
  readFileIntoStrings(trjFilename, trajInStrings);

  if(trajInStrings.size() < 10) {
  	std::cout << "ERROR: less than 10 tracks, quit" << std::endl;
  	return -1;
  }

  std::vector<Box> boxes = readBoundingBoxes(bbpath + std::to_string(vid) + ".txt");

  std::ofstream fout;
  fout.open(inpath + "scores_" + category + ".txt", std::ifstream::in | std::ofstream::out | std::ofstream::app);
  fout << threshold << ":" << countAverageTrajectoriesInBox(trajInStrings, boxes, threshold, clusterId, largestClusterId, largestClusterSize) << std::endl;
  fout.close();
  
return 0;  
}