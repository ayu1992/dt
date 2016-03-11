#include <unordered_map>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

const int TRACK_INFO_LEN = 10;
const int TRACK_LEN = 15;
const int HOG_DIM = 96;
const int HOF_DIM = 108;
const int MBHX_DIM = 96;
const int MBHY_DIM = 96;

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

