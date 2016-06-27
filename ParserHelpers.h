/**
 * Helper and utils functions.
 */
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

// Mappings used and explained in ParseTracks.cpp
std::map<std::string, int> subJHMDBActionClassMap {
  {"catch", 0},
  {"climb_stairs", 1},
  {"golf", 2},
  {"jump", 3},
  {"kick_ball", 4},
  {"pick", 5},
  {"pullup", 6},
  {"push", 7},
  {"run", 8},
  {"shoot_ball", 9},
  {"swing_baseball", 10},
  {"walk", 11}
};

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

std::map<std::string, int> kthActionClassMap {
  {"boxing", 0},
  {"handclapping", 1},
  {"handwaving", 2},
  {"running", 3},
  {"jogging", 4},
  {"walking", 5},
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

// Counts actual number of clusters after pspectral
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

// Reads and parses cluster assignments after pspectral
// returns a mapping: clusterId (trajectory id -> assigned cluster id)
std::unordered_map<int, int> readClusterId(const std::string& path) {
  int numActualClusters = countNonEmptyClusters(path);

  std::ifstream fin((path + "result.txt").c_str());
  if (!fin) {
    std::cerr << "Thrown by readClusterId, Unable to open file : " << std::endl;
  }

  std::unordered_map<int, int> tidToCid;
  std::unordered_map<int, int> resultIdToCid;  // Cid will contain exactly numActualClusters ids'.

  int resultId, trajIndex = 0;
  std::string line;
  int lastCid = 0;

  while (std::getline(fin, line)) {
    std::istringstream iss(line);
    iss >> resultId;  // resultId might be outside of range [0...numActualClusters]
    auto it = resultIdToCid.find(resultId); // try to find a previously defined cid
    if (it == resultIdToCid.end()) {
      resultIdToCid.insert({resultId, lastCid++});
    }

    tidToCid.insert({trajIndex, resultIdToCid.find(resultId)->second});
    trajIndex++;
  }
  
  fin.close();
  return tidToCid;
}

// Opens file and places contents into a temporary holder 'strings'
void readFileIntoStrings (const std::string& filename, std::vector<std::string>& strings) {
  std::string line;
  std::ifstream fin(filename.c_str());
  if (!fin) {
    std::cerr << "Unable to open file: " << filename << std::endl;
    return;  
  }
  while (std::getline(fin, line)) {
    strings.push_back(line);
  }
  fin.close();
  return;
}
