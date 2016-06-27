// Builds a graph where nodes are trajectories and 
// edges are the distances defined in Corso's paper[2].
// Outputs a symmetric matrix (_dij.txt)
#include <boost/functional/hash.hpp>
#include <cstdio>
#include <ctime>
#include <dirent.h>
#include <limits>
#include <random>

#include "BoostRelatedHelpers.h"

// Params defined in [2]
const float TAU_S = 16.0;
const int TAU_T = 8;

using Graph = std::unordered_map<std::pair<int, int>, float, boost::hash<std::pair<int, int>>>; 

// Computes educlidean distance between two 2D points
inline float spatialDistance(const point& p1, const point& p2) {
  point diff = p1 - p2;
  return sqrt(diff.x * diff.x + diff.y * diff.y);
}

// Normalizes coordinates by video frame dimension
inline std::vector<point> normalizeCoordsByFrame(
  const std::vector<point>& coords, 
  const int videoWidth, 
  const int videoHeight) {
  std::vector<point> temporary(coords.size(), point(0.0, 0.0));  // 16
  std::transform(coords.begin(), coords.end(), temporary.begin(), 
    [videoWidth, videoHeight](const point& p){
      return point(static_cast<int>(round(static_cast<float>(p.x) / videoWidth)), static_cast<int>(round(static_cast<float>(p.y) / videoHeight)));
    }
  );
    
  return temporary;  
}

inline float square(float f) { return f * f; }

// Generates graph
Graph generateGraph(
  const std::vector<track>& tracks, 
  const std::vector<std::vector<point>>& coords, 
  const float r) {
  
  Graph D;
  // NOTE: tracks were previously sorted in order by ending frames
  for(size_t traj_i = 0; traj_i < tracks.size(); traj_i++) {

    // find the range of traj_j within overlap
    const int endingFrame_i = tracks[traj_i].endingFrame;

    // binary search to find interval of track ids' within temporal overlap
    auto end_it = std::upper_bound(
      tracks.begin() + traj_i + 1, 
      tracks.end(),
      endingFrame_i,
      [](int endingFrame, const track& t) {
      return t.endingFrame > endingFrame + TRACK_LEN - TAU_T; }
    );

    for(size_t traj_j = traj_i + 1; traj_j < end_it - tracks.begin(); traj_j++) {
      
      // Ending frame indices for traj_i and traj_j
      const int endf_i = tracks[traj_i].endingFrame;
      const int endf_j = tracks[traj_j].endingFrame;

      const int offset = endf_j - endf_i;   // offset is 'o' in the paper   
      const int overlap = TRACK_LEN - offset;
      
      // Break early if not enough temporal overlap
      if(overlap < TAU_T) { 
        break;
      }

      // Compute d_ij
      float ds = 0.0;
      float dij = 0.0;
      for(int index_i = offset; index_i < TRACK_LEN; ++index_i) {
        ds += spatialDistance(coords[traj_i][index_i], coords[traj_j][index_i - offset]);
      }

      // Compute s_ij
      if((ds / overlap) < TAU_S) {     // equation 3 in paper
        //std::cout << "d : "<< d <<"; or: " << offset * r << "; or^2: "<< pow(offset * r, 2) << std::endl;
        dij = (ds + square(offset * r));
        D.emplace(std::make_pair(traj_i, traj_j), dij);
      }
    }
  } // end of Graph generation
  return D;
}

void printDistanceMatrix(const std::string& filename, const Graph& D, const int N) {
  
  // [[trj 0's neighbors], [trj 1's neighbors], ..., [trj N-1's]]
  std::vector<std::vector<std::pair<int, float>>> neighbors(N);

  // iterate every pair in D
  for (const auto& pair : D) {
      int trj_i = pair.first.first;
      int trj_j = pair.first.second;
      float dij = pair.second;
      neighbors[trj_i].emplace_back(trj_j, dij);
      neighbors[trj_j].emplace_back(trj_i, dij);
  } // end of iterating D

  for (int i = 0; i < N; i++) {
    neighbors[i].emplace_back(i, 0.00000000001);
  }

  // Output pspec id :track id
  // sort and print each list in neighbors 
  std::cout << "[BuildGraph] Opening output file : " << filename << std::endl;
  // C-style IO for faster performance
  // TODO: worth investigating other speed up methods as 
  // this step happens to be the most time consuming step in the entire project
  FILE* fout = fopen(filename.c_str(), "w");

  for(auto& v : neighbors) {
      // sort v's neighbors by their trajectory index
      std::sort(v.begin(), v.end(),
      [](const std::pair<int, float>& n1, const std::pair<int, float>& n2) {
        return n1.first < n2.first;}); 

      // file I/O
      for (size_t i = 0; i < v.size(); i++) {
        if( i == 0) fprintf(fout, "%d:%f", v[i].first, v[i].second);
        else  fprintf(fout, " %d:%f", v[i].first, v[i].second);
      }
      fprintf(fout, "\n");
  }
  return;
}

void outputSortedTrajectories(const std::string& outputPath, const std::vector<track>& tracks) {

  trackList tList;
  int index = 0;
  // from vector<track> --> vector<std::pair<int, track>>
  for (const auto & t : tracks) {
    tList.addTrack(index, t);
    index++;
  }
  
  std::ofstream ofs(outputPath + "_sortedTrajectories.out");
  {
      boost::archive::binary_oarchive oa(ofs);
      oa << tList;    // archive and stream closed when destructors are called
  }
}

int main(int argc, char** argv) {

  const std::string videoPath = argv[1];    // Location of trajectories
  const std::string outputPath = argv[2];   // Location to write sortedTrajectories
  const std::string videoName = argv[3];

  const float r = std::stof(argv[4]);
  const int MAX_NUM_TRACKS = std::stoi(argv[5]);

  videoRep video;
  restoreVideoRep(videoPath, video);

  // Discard dummy indices and just extract the trajectories
  std::vector<track> tracks;
  std::transform(
    video.getTrackList().tracks().begin(),
    video.getTrackList().tracks().end(), 
    std::back_inserter(tracks), 
    [](const std::pair<int, track>& p){
      return p.second;
    });
  
  std::cout << "[BuildGraph] " << tracks.size() << " trajectories in total" << std::endl;

  if (tracks.size() > MAX_NUM_TRACKS) {
    // Random sample some tracks 
    std::vector<track> samples = randomSample(tracks, MAX_NUM_TRACKS);
    tracks.clear();
    tracks = samples;
  }

  std::cout << "Processing " << tracks.size() << "tracks" << std::endl;

  // Sort Tracks by ending frame for ease of graph construction
  std::sort(
    tracks.begin(), 
    tracks.end(), 
    [](const track &a, const track &b) {
      return a.endingFrame < b.endingFrame;});

  // Assigns new track ids' in the process
  outputSortedTrajectories(outputPath + videoName, tracks);

  // Normalize the coordinates (only used in this file)
  std::cout << "[BuildGraph] Normalizing coordinates by frame size"<< std::endl;  

  std::vector<std::vector<point>> normalizedCoords;  // in identical order as tracks

  for (const auto& t : tracks) {
    normalizedCoords.push_back(normalizeCoordsByFrame(t.coords, video.videoWidth(), video.videoHeight()));
  }

  std::cout << "Generating graph, N2" << std::endl;
  Graph D = generateGraph(tracks, normalizedCoords, r);
  
  // Output s_ij for later spectral clustering
  printDistanceMatrix(outputPath + videoName + "_dij.txt", D, tracks.size());

  return 0;  
}
