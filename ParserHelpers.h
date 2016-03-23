#include <iostream>
#include <map>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

#include <boost/archive/tmpdir.hpp>
//#include <boost/archive/text_oarchive.hpp>
//#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/assume_abstract.hpp>

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

struct point {
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & x;
    ar & y;
  }
  float x;
  float y;
  
  point() {};
  point(const float x, const float y): x(x), y(y) {};

  // multiplication that mutates self
  point& operator*=(float s) {
    x *= s;
    y *= s;
    return *this;
  }
};

point operator+(const point& p1, const point& p2)
{
  return point(p1.x + p2.x, p1.y + p2.y);
}

point operator-(const point& p1, const point& p2)
{
  return point(p1.x - p2.x, p1.y - p2.y);
}

// p *= 3
point operator*(const point& p1, const float s)
{
  return point(p1.x * s, p1.y * s);
  // return point(p1) *= s;
}

struct track {
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & endingFrame & mean_x & mean_y & var_x & var_y;
    ar & length & scale & x_pos & y_pos & t_pos & displacements & coords;
    ar & hog & hof & mbhx & mbhy;
  }
  int endingFrame;
  float mean_x;
  float mean_y;
  float var_x;   // var of the coords
  float var_y;  
  float length;    // sum of Eucledian distance of Points on the trajectory
  float scale;   // The trajectory is computed on which scale
  float x_pos;
  float y_pos;
  float t_pos;
  std::vector<point> displacements;
  std::vector<point> coords;
  std::vector<float> hog;
  std::vector<float> hof;
  std::vector<float> mbhx;
  std::vector<float> mbhy;
  
  track() {};
  track(
    int endingFrame, float mean_x, float mean_y, float var_x, float var_y, float length, 
    float scale, float x_pos, float y_pos, float t_pos, std::vector<point> displacements, std::vector<point> coords, 
    std::vector<float> hog, std::vector<float> hof, std::vector<float> mbhx, 
    std::vector<float> mbhy) 
  : endingFrame(endingFrame), mean_x(mean_x), mean_y(mean_y), var_x(var_x), var_y(var_y), 
  length(length), scale(scale), x_pos(x_pos), y_pos(y_pos), t_pos(t_pos), displacements(displacements), coords(coords), 
  hog(hog), hof(hof), mbhx(mbhx), mbhy(mbhy) {}
};

class trackList {
private:
  friend class boost::serialization::access;
  friend std::ostream & operator<<(std::ostream &os, const trackList &tList);
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & _tracks;
  }

  std::vector<std::pair<int, track>> _tracks;

public: 
  trackList() {};
  trackList(const std::vector<std::pair<int, track>>& tracks): _tracks(tracks) {}
  
  /* TODO: track index should be handled by this class instead of caller*/
  void addTrack(int index, const track& t) {_tracks.emplace_back(index, t);}
  int size() const {return _tracks.size();}
  track getTrack(int index) const {return _tracks[index].second;}
  const std::vector<std::pair<int, track>>& tracks() const{ return _tracks;}
};

struct Box {
  point UpperLeft;
  float width;
  float height;
};

bool isInBox(const point& p, const Box& box) {
  float diffx = p.x - box.UpperLeft.x;
  float diffy = p.y - box.UpperLeft.y;
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
    boxes.emplace_back(Box{point(val[0], val[1]), val[2], val[3]});
  }
  return boxes;
}

void unnormalizePoints(std::vector<point>& points, const float trajectoryLength, const float mean_x, const float mean_y) {
  for (point& p : points) p = p * trajectoryLength;
  // Infer the last point.
  points.emplace_back(mean_x, mean_y);
  for (size_t i = 0; i < points.size() - 1; ++i) {
    points.back() = points.back() + (points[i] * (static_cast<float>(i + 1) / points.size()));
  }
  for (int i = points.size() - 2; i >= 0; --i) points[i] = points[i + 1] - points[i];
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
