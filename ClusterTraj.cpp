#include "DenseTrack.h"
#include "Initialize.h"
#include "Descriptors.h"
#include "Util.h"
#include "ParserHelpers.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <string>

// Dimension information to parse input file

const int TRACK_INFO_LEN = 10;
const int TRACK_LEN = 15;
const int HOG_DIM = 96;
const int HOF_DIM = 108;
const int MBHX_DIM = 96;
const int MBHY_DIM = 96;
const double TAU_S = 16.0;
const double r = 80;    // eq 4
using namespace cv;

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

void parseStringsToTrackTubes(
  const std::vector<std::string>& trajInStrings, const int L, std::vector<TrackTube>& trackTubes) {
  for (auto const& str: trajInStrings) {
    std::vector<float> val = split(str, '\t');

    // First 10 elems go inside TrackTubeInfo
    TrackTubeInfo tInfo = {
      static_cast<int>(val[0]), val[1], val[2], val[3], val[4], val[5], val[6], val[7], val[8], val[9]};
    
    // Construct Tracks
    // Construct Point2f objects
    std::vector<Point2f> coords;
    for (int i = TRACK_INFO_LEN; i < 2 * TRACK_LEN + 10; i+=2) {
        coords.emplace_back(val[i], val[i+1]);
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

    Track track = Track(coords, hog, hof, mbhx, mbhy);
    
    // Wrap it up with TrackTube
    trackTubes.emplace_back(TrackTube{tInfo, track});
  }
}

double spatialDistance(const Point2f& p1, const Point2f& p2) { 
  Point2f diff = p1 - p2;
  double dist = sqrt(diff.x * diff.x + diff.y * diff.y);
  return dist;
}

void printDistanceMatrix(const std::string& filename, const std::map<std::pair<int, int>, double>& D, const int N) {
  std::ofstream fout;
  std::cout << "Opening output file" << std::endl;
  fout.open(filename.c_str());
  // [[trj 0's neighbors], [trj 1's neighbors], ..., [trj N-1's]]
  std::vector<std::vector<std::pair<int, double>>> neighbors(N);

  std::cout << "Constructing adjacency lists" << std::endl;
  std::cout << "neighbors contains : " << neighbors.size() << "nodes" << std::endl;
  // iterate every pair in D
  for (const auto& pair : D) {
      int trj_i = pair.first.first;
      int trj_j = pair.first.second;
      double dij = pair.second;
      // TODO: neighbors[trj_i].emplace_back(trj_j, dij);
      neighbors[trj_i].emplace_back(trj_j, dij);
      neighbors[trj_j].emplace_back(trj_i, dij);

  } // end of iterating D

  std::cout << "Sort everyone's neighbors list and file I/O" << std::endl;
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

/**
 * Line format: trajectory id, ending frame number, scale, [(x1, y1), (x2, y2) ...]
 */
void printSortedTrajectories(const std::string& filename, const std::vector<TrackTube>& tracks) {
  std::ofstream fout;
  std::cout << "Opening output file" << std::endl;
  fout.open(filename.c_str());

  int trackId = 0;
  for(const auto& t : tracks) {
    fout << trackId << " " << t.trackTubeInfo.endingFrame << " " << t.trackTubeInfo.scale << " " << t.trackTubeInfo.length << " "
        << t.trackTubeInfo.mean_x << " " << t.trackTubeInfo.mean_y << " ";
    for(int i = 0; i < TRACK_LEN; i++) {
      fout << t.track.point[i].x << " " << t.track.point[i].y << " ";
    }
    fout << std::endl;
    trackId++;
  }
  fout.close();
  return;
}

int main() {
  std::vector<TrackTube> trackTubes; 
  std::vector<std::string> trajInStrings;

  // Read and parse feature dump
  std::cout << "Reading trajectories" << std::endl;
  readFileIntoStrings("out.features", trajInStrings);
  std::cout << "Parsing trajectories" << std::endl;
  parseStringsToTrackTubes(trajInStrings, TRACK_LEN, trackTubes); 
  std::cout << trackTubes.size() << " trajectories in total" << std::endl;
  
  std::cout << "Sorting trajectories by ending frame" << std::endl;
  // Sort all tracks by ending frame
  std::sort(
    trackTubes.begin(), 
    trackTubes.end(), 
    [](const TrackTube &a, const TrackTube &b) {
      return a.trackTubeInfo.endingFrame < b.trackTubeInfo.endingFrame;});

  // Output traj index, points

  // Generate Graph : d -> E -> S
  std::map<std::pair<int, int>, double> D;    // (traj index i, traj index j) -> s_ij

  std::cout << "Aligning trajectories and compute their distances" << std::endl;
  for(size_t traj_i = 0; traj_i < trackTubes.size(); traj_i++) {
    for(size_t traj_j = traj_i + 1; traj_j < trackTubes.size(); traj_j++) {
      
      // Ending frame indices for traj_i and traj_j
      const int endf_i = trackTubes[traj_i].trackTubeInfo.endingFrame;
      const int endf_j = trackTubes[traj_j].trackTubeInfo.endingFrame;
      const int offset = endf_j - endf_i;   // offset is 'o' in the paper

      double dij = 0.0;
      // Break early if doesn't overlap for 1 or more frames
      if(offset >= TRACK_LEN) { 
        // D.insert(std::pair<std::pair<int, int>, double>(std::pair<int, int>(traj_i, traj_j), dij));
        break;
      }
  
      // Compute d_ij
      double d = 0.0;
      const std::vector<Point2f>& coords_i = trackTubes[traj_i].track.point;
      const std::vector<Point2f>& coords_j = trackTubes[traj_j].track.point;
      for(int index_i = offset; index_i < TRACK_LEN; index_i++) {
        d += spatialDistance(coords_i[index_i], coords_j[index_i - offset]);
      }

      // Compute s_ij
      if(d / (TRACK_LEN - offset) < TAU_S) {     // equation 3 in paper
        // TODO: Maybe filter by TAU_T too. Just something to think about.
        dij = (d + pow(offset * r, 2));
        D.emplace(std::make_pair(traj_i, traj_j), dij);
      }
    }
  } // end of Graph generation

  // Spectral clustering : in pspectralclustering folder
  std::cout << "Outputing distance matrix" << std::endl;
  printDistanceMatrix("dij.txt", D, trackTubes.size());

  std::cout << "Outputing sorted trajectories" << std::endl;
  // dump trajectory -> trj_id ending frame, scale, points
  printSortedTrajectories("sortedTrajectories.txt", trackTubes);

return 0;  
}