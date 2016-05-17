#include "BoostRelatedHelpers.h"
#include <ctime>
#include <dirent.h>
#include <limits>
#include <random>

#include <boost/functional/hash.hpp>
#include <cstdio>

/* TODO: functional and file documentation */
/* Read *.features, output sortedTrajectories */
// Dimension information to parse input file
const float TAU_S = 16.0;
const int TAU_T = 8;
const int MAX_NUM_TRACKS = 8000;

using Graph = std::unordered_map<std::pair<int, int>, float, boost::hash<std::pair<int, int>>>; 

inline float spatialDistance(const point& p1, const point& p2) {
  point diff = p1 - p2;
  return sqrt(diff.x * diff.x + diff.y * diff.y);
}
  
inline std::vector<point> normalizeCoordsByFrame(const std::vector<point>& coords, const int videoWidth, const int videoHeight) {
  std::vector<point> temporary(coords.size(), point(0.0, 0.0));  // 16
  std::transform(coords.begin(), coords.end(), temporary.begin(), 
    [videoWidth, videoHeight](const point& p){
      return point(static_cast<int>(round(static_cast<float>(p.x) / videoWidth)), static_cast<int>(round(static_cast<float>(p.y) / videoHeight)));
    }
  );
    
  return temporary;  
}

inline float square(float f) { return f * f; }

// Generate Graph : d -> E -> S
// tracks were sorted in order by ending frames
Graph generateGraph(const std::vector<track>& tracks, const std::vector<std::vector<point>>& coords, const float r) {
  
  Graph D;

  for(size_t traj_i = 0; traj_i < tracks.size(); traj_i++) {

    // find the range of traj_j within overlap
    const int endingFrame_i = tracks[traj_i].endingFrame;

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
      
      // Break early if not enough overlap
      if(overlap < TAU_T) { 
        break;
      }

      // Compute d_ij
      float ds = 0.0;
      float dij = 0.0;
      for(int index_i = offset; index_i < TRACK_LEN; ++index_i) {
        //d += spatialDistance(tracks[traj_i].displacements[index_i], tracks[traj_j].displacements[index_i - offset]);
        ds += spatialDistance(coords[traj_i][index_i], coords[traj_j][index_i - offset]);
      }
     // std::cout << "hi" << std::endl;
      // Compute s_ij
      if((ds / overlap) < TAU_S) {     // equation 3 in paper
        //std::cout << "d : "<< d <<"; or: " << offset * r << "; or^2: "<< pow(offset * r, 2) << std::endl;
        dij = (ds + square(offset * r));
        //dij = d + offset * r;
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
  // save data to archive
  {
      boost::archive::binary_oarchive oa(ofs);
      oa << tList;    // archive and stream closed when destructors are called
  }
}

int main(int argc, char** argv) {

  std::string videoPath = argv[1];    // Location of trajectories
  std::string outputPath = argv[2];   // Location to write sortedTrajectories
  std::string videoName = argv[3];

  float r = std::stof(argv[4]);

  // Read and pack feature dump into Tracks(temporary container)
  std::vector<track> temporary; 
  std::vector<std::string> trajInStrings;
  int videoWidth, videoHeight;
  parseFeaturesToTracks(videoPath, trajInStrings, temporary, videoWidth, videoHeight); 
  std::cout << "[BuildGraph] "<< temporary.size() << " trajectories in total" << std::endl;

  std::vector<track> tracks;

  if (temporary.size() > MAX_NUM_TRACKS) {
    // Random sample some tracks 
    std::random_shuffle(temporary.begin(), temporary.end());
    std::vector<track> samples(temporary.begin(), temporary.begin() + MAX_NUM_TRACKS);
    tracks = samples;
  } else {
    tracks = temporary;
  }
    std::cout << tracks.size() << "tracks" << std::endl;

  // Sort Tracks by ending frame for ease of graph construction
  std::sort(
    tracks.begin(), 
    tracks.end(), 
    [](const track &a, const track &b) {
      return a.endingFrame < b.endingFrame;});

  outputSortedTrajectories(outputPath+videoName, tracks);

  // Normalize the coordinates (only used in this file)
  std::cout << "[BuildGraph] Normalizing coordinates by frame size"<< std::endl;  

  std::vector<std::vector<point>> normalizedCoords;  // in identical order as tracks

  for (const auto& t : tracks) {
    normalizedCoords.push_back(normalizeCoordsByFrame(t.coords, videoWidth, videoHeight));
  }

  // (traj index i, traj index j) -> s_ij
  std::cout << "Generating graph, N2" << std::endl;
  Graph D = generateGraph(tracks, normalizedCoords, r);
  // [[trj 0's neighbors], [trj 1's neighbors], ..., [trj N-1's]]
  
  // Output s_ij for spectral clustering
  printDistanceMatrix(outputPath + videoName + "_dij.txt", D, tracks.size());

  return 0;  
}
