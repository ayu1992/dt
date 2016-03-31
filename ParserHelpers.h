#include <iostream>
#include <map>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>

const int TRACK_LEN = 15;
const int HOG_DIM = 96;
const int HOF_DIM = 108;
const int MBHX_DIM = 96;
const int MBHY_DIM = 96;

std::map<std::string, int> actionClassMap {
  {"Diving-Side", 0},
  {"Golf-Swing-Back", 1},
  {"Golf-Swing-Side", 2},
  {"Kicking-Front", 3},
  {"Kicking-Side", 4},
  {"Lifting", 5},
  {"Riding-Horse", 6},
  {"Run-Side", 7},
  {"SkateBoarding-Front", 8},
  {"Swing-Bench", 9},
  {"Swing-SideAngle", 10},
  {"Walk-Front", 11},
  {"Golf-Swing-Front", 12}
};

// Splits a string into tokens by delim character
std::vector<float> split(const std::string& str, char delim) {
	std::vector<float> elems;
	std::stringstream ss(str);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(::atof(item.c_str()));
	}
	return elems;
}

// traj id -> cid
void readClusterId(const std::string& filename, std::unordered_map<int, int>& clusterId) {
  std::string line;
  std::ifstream fin(filename.c_str());
  if (!fin) {
    std::cerr << "Unable to open file : " << filename << std::endl;
    return;  
  }

  int cid, trajIndex = 0;
  while (std::getline(fin, line)) {
    std::istringstream iss(line);
    iss >> cid;
    clusterId.insert({trajIndex, cid});
    trajIndex++;
  }
  
  fin.close();
  return;
}

int returnIdOfLargestCluster(const std::string& idFilename, std::unordered_map<int, int>& clusterId) {
    // Read cluster ids'
  readClusterId(idFilename, clusterId);

  std::map<int, int> clusterSizes;
  // Identify the biggest cluster (cid)   
  for (const auto& trjIdCidPair: clusterId) {
    //clusterSizes[pair.second] += 1;
    clusterSizes[trjIdCidPair.second]++;
  }

  // TODO: Maybe change arg type of the lambda to "const std::pair<int, size_t>&".
  auto maxElemIter = std::max_element(
    clusterSizes.begin(), clusterSizes.end(), 
    [] (const std::pair<int, int>& p1, const std::pair<int, int>& p2) {
        return p1.second < p2.second;
    });

  std::cout << "Largest cluster contains " << maxElemIter->second << " tracks" << std::endl;
  return maxElemIter->first;
}

void readFileIntoStrings (const std::string& filename, std::vector<std::string>& strings) {
  std::string line;
  std::ifstream fin(filename.c_str());
  if (!fin) {
    std::cerr << "Unable to open file" << std::endl;
    return;  
  }
  while (std::getline(fin, line)) {
    strings.push_back(line);
  }
  fin.close();
  return;
}
