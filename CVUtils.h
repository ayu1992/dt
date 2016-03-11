#include <unordered_map>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>

struct Box {
  Point2f UpperLeft;
  float width;
  float height;
};

bool isInBox(const Point2f& point, const Box& box) {
  float diffx = point.x - box.UpperLeft.x;
  float diffy = point.y - box.UpperLeft.y;
  return (diffx >= 0 && diffx <= box.width) && (diffy >= 0 && diffy <= box.height);
}

std::vector<Box> readBoundingBoxes(std::string filepath) {
  // read the boxes frame by frame
  std::string line;
  std::ifstream fin(filepath.c_str());
  std::vector<Box> boxes;
  if (!fin) {
    std::cerr << "Unable to open file : " << filepath << std::endl;
    return boxes;
  } 
  while(std::getline(fin, line)) {
    std::vector<float> val = split(line, ' ');
    boxes.emplace_back(Box{Point2f(val[0], val[1]), val[2], val[3]});
  }
  return boxes;
}

/**
 * Each line in the file corresponds to a trajectory.
 */
void readFileIntoStrings (const std::string& filename, std::vector<std::string>& trajInStrings) {
  std::string line;
  std::ifstream fin(filename.c_str());
  if (!fin) {
    std::cerr << "Unable to open file" << std::endl;
    return;  
  }
  while (std::getline(fin, line)) {
      trajInStrings.push_back(line);
  }
  fin.close();
  return;
}

void parseLineFromSortedTrajectories(const std::string& line, int& trackID, int& endingFrame, float& scale, float& trajectoryLength, float& mean_x, float& mean_y, std::vector<Point2f>& points) {
	std::vector<float> val = split(line, ' ');
	trackID = static_cast<int>(val[0]);
	endingFrame = static_cast<int>(val[1]);
	scale = val[2];
	trajectoryLength = val[3];
	mean_x = val[4];
	mean_y = val[5];
	for(int i = 6; i < 6 + 30; i += 2) {
		points.push_back(Point2f(val[i], val[i+1]));
	}
}

void unnormalizePoints(std::vector<Point2f>& points, const float trajectoryLength, const float mean_x, const float mean_y) {
	for (Point2f& p : points) p *= trajectoryLength;
	// Infer the last point.
	points.emplace_back(mean_x, mean_y);
	for (size_t i = 0; i < points.size() - 1; ++i) {
		points.back() += (points[i] * (static_cast<float>(i + 1) / points.size()));
	}
	for (int i = points.size() - 2; i >= 0; --i) points[i] = points[i + 1] - points[i];
}

// Returns a random color
Scalar getRandomColor(void) {
  int r = rand() % 255;
    r = (r + 200) / 2;
    int g = rand() % 255;
    g = (g + 200) / 2;
    int b = rand() % 255;
    b = (b + 200) / 2;
    return Scalar(r,g,b);
}

int returnIdOfLargestCluster(const std::string& idFilename, std::unordered_map<int, int>& clusterId, std::vector<int>& clusterSizes) {
    // Read cluster ids'
  readClusterId(idFilename, clusterId);

  // Identify the biggest cluster (cid)   
  for (const auto& pair: clusterId) {
    clusterSizes[pair.second] += 1;
  }

  // TODO: Maybe change arg type of the lambda to "const std::pair<int, size_t>&".
  auto maxElemIter = std::max_element(clusterSizes.begin(), clusterSizes.end());
  return std::distance(clusterSizes.begin(), maxElemIter);
} 