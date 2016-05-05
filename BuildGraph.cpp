#include "BoostRelatedHelpers.h"
/* Read *.features, output sortedTrajectories */
// Dimension information to parse input file
const double TAU_S = 16.0;
const int TAU_T = 8;

// Generate Graph : d -> E -> S
// tracks were sorted in order by ending frames
std::map<std::pair<int, int>, double> generateGraph(const std::vector<track>& tracks, const float r, const int videoWidth, const int videoHeight) {
  std::map<std::pair<int, int>, double> D;

  auto spatialDistance = [](const point& p1, const point& p2)-> float {
    point diff = p1 - p2;
    return sqrt(diff.x * diff.x + diff.y * diff.y);
  };

  auto normalizeByDimension = [videoWidth, videoHeight](const point& p)-> point {
    return point(static_cast<int>(round(static_cast<float>(p.x) / videoWidth)), static_cast<int>(round(static_cast<float>(p.y) / videoHeight)));
  };

  for(size_t traj_i = 0; traj_i < tracks.size(); traj_i++) {
    for(size_t traj_j = traj_i + 1; traj_j < tracks.size(); traj_j++) {
      
      // Ending frame indices for traj_i and traj_j
      const int endf_i = tracks[traj_i].endingFrame;
      const int endf_j = tracks[traj_j].endingFrame;
      const int offset = endf_j - endf_i;   // offset is 'o' in the paper

      double dij = 0.0;
      // Break early if doesn't overlap for 1 or more frames
      if(offset > TRACK_LEN) { 
        break;
      }
    
      // Filter by TAU_T, overlap must be greater than threshold
      const int overlap = TRACK_LEN - offset;
      if (overlap <= TAU_T) {
        continue;
      }

      std::vector<point> xi;
      for (const auto& p : tracks[traj_i].coords) {
        xi.push_back(normalizeByDimension(p));
      }

      std::vector<point> xj;
      for (const auto& p : tracks[traj_j].coords) {
        xj.push_back(normalizeByDimension(p));
      }
      
      // Compute d_ij
      double d = 0.0;
      for(int index_i = offset; index_i < TRACK_LEN; ++index_i) {
        //d += spatialDistance(tracks[traj_i].displacements[index_i], tracks[traj_j].displacements[index_i - offset]);
        d += spatialDistance(xi[index_i], xj[index_i - offset]);
      }

      // Compute s_ij
      if((d / overlap) < TAU_S) {     // equation 3 in paper
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
  std::cout << "[BuildGraph] Opening output file : " << filename << std::endl;
  fout.open(filename, std::ofstream::out);
  // [[trj 0's neighbors], [trj 1's neighbors], ..., [trj N-1's]]
  std::vector<std::vector<std::pair<int, double>>> neighbors(N);

  // iterate every pair in D
  for (const auto& pair : D) {
      int trj_i = pair.first.first;
      int trj_j = pair.first.second;
      double dij = pair.second;
      neighbors[trj_i].emplace_back(trj_j, dij);
      neighbors[trj_j].emplace_back(trj_i, dij);
  } // end of iterating D

  for (int i = 0; i < N; i++) {
    neighbors[i].emplace_back(i, 0.00000000001);
  }
  // Output pspec id :track id
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

int main(int argc, char** argv) {

  std::string videoPath = argv[1];    // Location of trajectories
  std::string outputPath = argv[2];   // Location to write sortedTrajectories
  std::string videoName = argv[3];

  float r;
  std::istringstream getGamma(argv[4]);
  getGamma >> r;

  // Read and pack feature dump into Tracks(temporary container)
  std::vector<track> tracks; 
  std::vector<std::string> trajInStrings;
  int videoWidth, videoHeight;
  parseFeaturesToTracks(videoPath, trajInStrings, tracks, videoWidth, videoHeight); 
  std::cout << "[BuildGraph] "<< tracks.size() << " trajectories in total" << std::endl;

  // Sort Tracks by ending frame for ease of graph construction
  std::sort(
    tracks.begin(), 
    tracks.end(), 
    [](const track &a, const track &b) {
      return a.endingFrame < b.endingFrame;});

  // (traj index i, traj index j) -> s_ij
  std::map<std::pair<int, int>, double> D = generateGraph(tracks, r, videoWidth, videoHeight);

  // [[trj 0's neighbors], [trj 1's neighbors], ..., [trj N-1's]]
  
  // Output s_ij for spectral clustering
  printDistanceMatrix(outputPath + videoName + "_dij.txt", D, tracks.size());
  
  trackList tList;
  int index = 0;
  // from vector<track> --> vector<std::pair<int, track>>
  for (const auto & t : tracks) {
    tList.addTrack(index, t);
    index++;
  }
  
  std::ofstream ofs(outputPath + videoName + "_sortedTrajectories.out");
  // save data to archive
  {
      boost::archive::binary_oarchive oa(ofs);
      oa << tList;    // archive and stream closed when destructors are called
  }
  return 0;  
}
