#include "ParserHelpers.h"
/* Read *.features, output sortedTrajectories */
// Dimension information to parse input file
const double TAU_S = 16.0;
const int TAU_T = 8;

const int TRACK_INFO_LEN = 10;

void parseStringsToTracks(
  const std::vector<std::string>& trajInStrings, const int L, std::vector<track>& tracks) {
  for (auto const& str: trajInStrings) {
    std::vector<float> val = split(str, '\t');

    std::vector<point> displacements;
    for (int i = TRACK_INFO_LEN; i < 2 * TRACK_LEN + 10; i += 2) {
      displacements.push_back(point(val[i], val[i+1]));
    }

    std::vector<point> coords(displacements);
    unnormalizePoints(coords, val[5], val[1], val[2]);

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
      track(static_cast<int>(val[0]), val[1], val[2], val[3], val[4], val[5], val[6], val[7], val[8], val[9], displacements, coords, hog, hof, mbhx, mbhy));
  }
}

// Generate Graph : d -> E -> S
// tracks were sorted in order by ending frames
std::map<std::pair<int, int>, double> generateGraph(const std::vector<track>& tracks, const float r) {
  std::map<std::pair<int, int>, double> D;

  auto spatialDistance = [](const point& p1, const point& p2)-> float {
    point diff = p1 - p2;
    return sqrt(diff.x * diff.x + diff.y * diff.y);
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

      // Compute d_ij
      double d = 0.0;
      for(int index_i = offset; index_i < TRACK_LEN; ++index_i) {
        d += spatialDistance(tracks[traj_i].coords[index_i], tracks[traj_j].coords[index_i - offset]);
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
  std::cout << "[ClusterTracks] Opening output file : " << filename << std::endl;
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

  // Sort Tracks by ending frame for ease of graph construction
  std::sort(
    tracks.begin(), 
    tracks.end(), 
    [](const track &a, const track &b) {
      return a.endingFrame < b.endingFrame;});

  // (traj index i, traj index j) -> s_ij
  std::map<std::pair<int, int>, double> D = generateGraph(tracks, r);

  /* Some nodes are isolated, remove them; we'll only be looking at the edge set */
  // [[trj 0's neighbors], [trj 1's neighbors], ..., [trj N-1's]]
  
  /*std::vector<std::vector<std::pair<int, double>>> neighbors(N);

  // iterate every pair in D
  for (const auto& pair : D) {
      int trj_i = pair.first.first;
      int trj_j = pair.first.second;
      double dij = pair.second;
      neighbors[trj_i].emplace_back(trj_j, dij);
      neighbors[trj_j].emplace_back(trj_i, dij);
  } // end of iterating D
*/
  // Output s_ij for spectral clustering
  printDistanceMatrix(dumppath + "dij.txt", D, tracks.size());
  
  trackList tList;
  int index = 0;
  // from vector<track> --> vector<std::pair<int, track>>
  for (const auto & t : tracks) {
    tList.addTrack(index, t);
    index++;
  }
  
  std::ofstream ofs(dumppath + "sortedTrajectories.out");
  // save data to archive
  {
      //boost::archive::text_oarchive oa(ofs);
      boost::archive::binary_oarchive oa(ofs);
      oa << tList;    // archive and stream closed when destructors are called
  }
  return 0;  
}