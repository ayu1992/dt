#include <iostream>
#include <map>
#include <unordered_map>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>

const int TRACK_LEN = 15;
const int TRACK_INFO_LEN = 12;
const int HOG_DIM = 96;
const int HOF_DIM = 108;
const int MBHX_DIM = 96;
const int MBHY_DIM = 96;

std::map<std::string, int> ucfActionClassMap {
  {"Diving-Side", 0},
  {"Golf-Swing-Back", 1},
  {"Golf-Swing-Side", 1},
  {"Kicking-Front", 2},
  {"Kicking-Side", 2},
  {"Lifting", 3},
  {"Riding-Horse", 4},
  {"Run-Side", 5},
  {"SkateBoarding-Front", 6},
  {"Swing-Bench", 7},
  {"Swing-SideAngle", 8},
  {"Walk-Front", 9},
  {"Golf-Swing-Front", 1}
};

std::map<std::string, int> OlympicActionClassMap {
  {"bowling", 0},
  {"triple_jump", 1},
  {"pole_vault", 2},
  {"diving_platform", 3},
  {"hammer_throw", 4},
  {"tennis_serve", 5},
  {"snatch", 6},
  {"vault", 7},
  {"clean_and_jerk", 8},
  {"basketball_layup", 9},
  {"high_jump", 10},
  {"shot_put", 11},
  {"long_jump", 12},
  {"discus_throw", 13},
  {"diving_springboard", 14},
  {"javelin_throw", 15}
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
/*To be deprecated*/
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

int countNonEmptyClusters(const std::string path) {
  std::ifstream fin((path + "result.txt").c_str());
  std::set<int> dedup;
  std::string line;
  while(std::getline(fin, line)) {
    int cid = std::stoi(line); 
    dedup.insert(cid);
  }
  return dedup.size();
}

// clusterId : trajId -> cid
std::unordered_map<int, int> readClusterId(const std::string path) {
  std::ifstream fin((path + "result.txt").c_str());
  if (!fin) {
    std::cerr << "Thrown by readClusterId, Unable to open file : " << std::endl;
  }

  std::unordered_map<int, int> clusterId;
  int cid, trajIndex = 0;
  std::string line;
  while (std::getline(fin, line)) {
    std::istringstream iss(line);
    iss >> cid;
    clusterId.insert({trajIndex, cid});
    trajIndex++;
  }
  
  fin.close();
  return clusterId;
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