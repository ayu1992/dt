#include <algorithm>
#include <cmath>
#include <fstream>
#include <string>
#include "ParserHelpers.h"
#include <boost/archive/tmpdir.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/assume_abstract.hpp>

/* Read *.features, output sortedTrajectories */

// Dimension information to parse input file
const double TAU_S = 16.0;
const int TAU_T = 8;

const int TRACK_INFO_LEN = 10;

class point {
private: 
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & x;
    ar & y;
  }
  float x;
  float y;
public:
  point() {};
  point(const float x, const float y): x(x), y(y) {};
  float getX(void) {return x;}
  float getY(void) {return y;}
};

point operator+(point &p1, point &p2)
{
  return point(p1.getX() + p2.getX(), p1.getY() + p2.getY());
}

point operator-(point &p1, point &p2)
{
  return point(p1.getX() - p2.getX(), p1.getY() - p2.getY());
}

class track {
private:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & endingFrame;
    ar & mean_x;
    ar & mean_y;
    ar & var_x;
    ar & var_y;
    ar & length;
    ar & scale;
    ar & x_pos;
    ar & y_pos;
    ar & t_pos;
    ar & coords;
    ar & hog;
    ar & hof;
    ar & mbhx;
    ar & mbhy;
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
  std::vector<point> coords;
  std::vector<float> hog;
  std::vector<float> hof;
  std::vector<float> mbhx;
  std::vector<float> mbhy;
public:
  track() {};
  
  track(
    int endingFrame, float mean_x, float mean_y, float var_x, float var_y, float length, 
    float scale, float x_pos, float y_pos, float t_pos, std::vector<point> coords, 
    std::vector<float> hog, std::vector<float> hof, std::vector<float> mbhx, 
    std::vector<float> mbhy) 
  : endingFrame(endingFrame), mean_x(mean_x), mean_y(mean_y), var_x(var_x), var_y(var_y), 
  length(length), scale(scale), x_pos(x_pos), y_pos(y_pos), t_pos(t_pos), coords(coords), 
  hog(hog), hof(hof), mbhx(mbhx), mbhy(mbhy) {}
  
  int getEndingFrame() {return endingFrame;}
  float getMean_x() { return mean_x;}
  float getMean_y() { return mean_y;}
  float getVar_x() { return var_x;}
  float getVar_y() { return var_y;}
  float getLength() {return length;}
  float getScale() { return scale;}
  float getXPos() {return x_pos;}
  float getYPos() {return y_pos;}
  float getTPos() {return t_pos;}
  
  std::vector<point>& getMutableCoords() {return coords;}
  std::vector<float>& getMutableHog() {return hog;}
  std::vector<float>& getMutableHof() {return hof;}
  std::vector<float>& getMutableMBH_X() {return mbhx;} 
  std::vector<float>& getMutableMBH_Y() {return mbhy;} 

  const std::vector<point>& getCoords() {return coords;}
};

class trackList {
private:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & tracks;
  }

  std::vector<track> tracks;

public: 
  trackList() {};
  trackList(std::vector<track> tracks): tracks(tracks) {}
  
  void addTrack(track& t) {tracks.push_back(t);}
  int size(void) {return tracks.size();}
  track getTrack(int index) {return tracks[index];}
};

void parseStringsToTracks(
  const std::vector<std::string>& trajInStrings, const int L, std::vector<track>& tracks) {
  for (auto const& str: trajInStrings) {
    std::vector<float> val = split(str, '\t');

    std::vector<point> coords;
    for (int i = TRACK_INFO_LEN; i < 2 * TRACK_LEN + 10; i+=2) {
      coords.push_back(point(val[i], val[i+1]));
    }

    // Construct Descriptor objects
    std::vector<float>::iterator hogIteratorBegin = val.begin() + TRACK_INFO_LEN + 2 * TRACK_LEN;
    std::vector<float> hog(hogIteratorBegin, hogIteratorBegin + HOG_DIM);
    std::vector<float>::iterator hofIteratorBegin = hogIteratorBegin + HOG_DIM;
    std::vector<float> hof(hogIteratorBegin, hogIteratorBegin + HOG_DIM);

    std::vector<float>::iterator mbhxIteratorBegin = hofIteratorBegin + HOF_DIM;
    std::vector<float> mbhx(mbhxIteratorBegin, mbhxIteratorBegin + MBHX_DIM);

    std::vector<float>::iterator mbhyIteratorBegin = mbhxIteratorBegin + MBHX_DIM;
    std::vector<float> mbhy(mbhyIteratorBegin, mbhyIteratorBegin + MBHY_DIM);

    // Construct Tracks
    tracks.push_back(
      track(static_cast<int>(val[0]), val[1], val[2], val[3], val[4], val[5], val[6], val[7], val[8], val[9], coords, hog, hof, mbhx, mbhy));
  }
}

// Generate Graph : d -> E -> S
// tracks were sorted in order by ending frames
std::map<std::pair<int, int>, double> generateGraph(const std::vector<track>& tracks, const float r) {
  std::map<std::pair<int, int>, double> D;

  auto spatialDistance = [](const point& p1, const point& p2)-> float {
    point diff = p1 - p2;
    return sqrt(diff.getX() * diff.getX() + diff.getY() * diff.getY());
  };

  for(size_t traj_i = 0; traj_i < tracks.size(); traj_i++) {
    for(size_t traj_j = traj_i + 1; traj_j < tracks.size(); traj_j++) {
      
      // Ending frame indices for traj_i and traj_j
      const int endf_i = tracks.getTrack(traj_i).getEndingFrame();
      const int endf_j = tracksgetTrack(traj_j).getEndingFrame();
      const int offset = endf_j - endf_i;   // offset is 'o' in the paper

      double dij = 0.0;
      // Break early if doesn't overlap for 1 or more frames
      if(offset >= TRACK_LEN) { 
        break;
      }
    
      // Filter by TAU_T, overlap must be greater than threshold
      const int overlap = TRACK_LEN - offset;
      if (overlap <= TAU_T) {
        continue;
      }

      // Compute d_ij
      double d = 0.0;
      for(int index_i = offset; index_i < TRACK_LEN; ++index_i) {
        d += spatialDistance(tracks.getTrack(traj_i).getCoords()[index_i], tracks.getTrack(traj_j).getCoords()[index_i - offset]);
      }

      // Compute s_ij
      if(d / overlap < TAU_S) {     // equation 3 in paper
        //std::cout << "d : "<< d <<"; or: " << offset * r << "; or^2: "<< pow(offset * r, 2) << std::endl;
        dij = (d + pow(offset * r, 2));
        //dij = d + offset * r;
        D.emplace(std::make_pair(traj_i, traj_j), dij);
      }
    }
  } // end of Graph generation
  return D;
}

void printDistanceMatrix(const std::string& filename, const std::map<std::pair<int, int>, double>& D, const int N) {
  std::ofstream fout;
  std::cout << "[ClusterTracks] Opening output file : " << filename << std::endl;
  fout.open(filename, std::ofstream::out);
  // [[trj 0's neighbors], [trj 1's neighbors], ..., [trj N-1's]]
  std::vector<std::vector<std::pair<int, double>>> neighbors(N);

//  std::cout << "[ClusterTracks] Constructing adjacency lists" << std::endl;
//  std::cout << "[ClusterTracks] neighbors contains : " << neighbors.size() << " nodes" << std::endl;
  // iterate every pair in D
  for (const auto& pair : D) {
      int trj_i = pair.first.first;
      int trj_j = pair.first.second;
      double dij = pair.second;
      neighbors[trj_i].emplace_back(trj_j, dij);
      neighbors[trj_j].emplace_back(trj_i, dij);

  } // end of iterating D

// std::cout << "[ClusterTracks] Sort everyone's neighbors list and file I/O" << std::endl;
  // sort and print each list in neighbors
  for(auto& v : neighbors) {
      // sort v's neighbors by their trajectory index
      std::sort(v.begin(), v.end(),
      [](const std::pair<int, double>& n1, const std::pair<int, double>& n2) {
        return n1.first < n2.first;});

      // file I/O
      for (size_t i = 0; i < v.size(); i++) {
        if( i != 0) fout << " ";  
        fout << v[i].first << ":" << v[i].second;
      }
      fout << std::endl;
  }
  fout.close();
  return;
}


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

int main(int argc, char** argv) {

  std::string inpath = argv[1];
  std::string dumppath = argv[2];

  float r;
  std::istringstream getGamma(argv[3]);
  getGamma >> r;

  std::vector<track> tracks; 
  std::vector<std::string> trajInStrings;

  // Read and pack feature dump into Tracks(temporary container)
  readFileIntoStrings(inpath, trajInStrings);
  
  parseStringsToTracks(trajInStrings, TRACK_LEN, tracks); 
  std::cout << "[ClusterTracks] "<< tracks.size() << " trajectories in total" << std::endl;

  // Sort Tracks by ending frame for easy of graph construction
  std::sort(
    tracks.begin(), 
    tracks.end(), 
    [](const track &a, const track &b) {
      return a.getEndingFrame() < b.getEndingFrame();});

  trackList tList(tracks);
  std::ofstream ofs(dumppath + "sortedTrajectories.out");
  // save data to archive
  {
      boost::archive::text_oarchive oa(ofs);
      oa << tList;
    // archive and stream closed when destructors are called
  }

  // (traj index i, traj index j) -> s_ij
  std::map<std::pair<int, int>, double> D = generateGraph(tracks, r);

  // Output s_ij for spectral clustering
  printDistanceMatrix(dumppath + "dij.txt", D, tracks.size());
  return 0;  
    }
/*
  */