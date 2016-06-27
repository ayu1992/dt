/**
 * Reads cluster assignment and trajectories
 * Computes super trajectories by merging tracks in the same cluster
 */
// I'm computing full matrices here for ease of debugging
// you may consider just computing only the top triangles to enhance the performance.
#include <algorithm>
#include <cstdio>
#include <functional>
#include <string>
#include <sstream>
#include <utility>

#include <boost/algorithm/string.hpp>
#include "BoostRelatedHelpers.h"

using Edges = std::vector<std::vector<float>>;		// Express graph edges in terms of a KxK matrix

// Adding two vectors by performing plus operation on each element
template <typename T>
void vectorAdd(const std::vector<T>& v1, std::vector<T>& v2) {
	std::transform(v1.begin(), v1.end(), v2.begin(), v2.begin(), std::plus<T>());
}

// Add newTrack to targetTracks
void addToTrack(const track& newTrack, track& targetTrack) {	
	vectorAdd(newTrack.displacements, targetTrack.displacements);
	vectorAdd(newTrack.coords, targetTrack.coords);
	vectorAdd(newTrack.hog, targetTrack.hog);
	vectorAdd(newTrack.hof, targetTrack.hof);
	vectorAdd(newTrack.mbhx, targetTrack.mbhx);
	vectorAdd(newTrack.mbhy, targetTrack.mbhy);
}

// A functor that computes average values for a type of feature
struct TakeAverageForFeature {
  const float numTracksAdded;
  TakeAverageForFeature(int numTracksAdded) : numTracksAdded(numTracksAdded) {}

  template <typename T>
  void operator()(std::vector<T>& v) const {
  	std::transform(v.begin(), v.end(), v.begin(), [this](const T& e){ return e / numTracksAdded; });
  }
};

// Computes supertrack representation for a group of tracks.
// Here we employ a simple averaging strategy on each channel.
track computeSuperTrackFromGroup(const std::vector<track>& tracks) {

	track superTrack(tracks[0]);	// tracks remains sorted by endingFrames
	int numTracksAdded = 0;
	for (const auto& t : tracks) {
		addToTrack(t, superTrack);
		++numTracksAdded;
	}

	// take average and statistics
	if (numTracksAdded) {
		auto take_average_for_feature = TakeAverageForFeature(numTracksAdded);
		take_average_for_feature(superTrack.displacements);
		take_average_for_feature(superTrack.coords);
		take_average_for_feature(superTrack.hog);
		take_average_for_feature(superTrack.hof);
		take_average_for_feature(superTrack.mbhx);
		take_average_for_feature(superTrack.mbhy);
	}

	// recalculate means, var if neccessary
	checkContainsEmptyFeature(superTrack);
	return superTrack;
}

// Given a list of trajecotries, reads everyone's cluster assigment
// and computes a super track for each cluster.
// clusterId : a mapping from trajectoryId -> assigned cluster id
// tList is sorted in endingFrame by ascending order
trackList computeSuperTracks(
	const int numClusters, 
	const std::unordered_map<int, int>& clusterId, 
	const trackList& tList) {
	
	trackList superTracks;			// rvo
	
	// put the trajectories to their respective clusters
	std::vector<std::vector<track>> groups(numClusters, std::vector<track>());
	for (const auto& idTrackPair : tList.tracks()) {	// pair<track id, track>
		int cid = clusterId.find(idTrackPair.first)->second;	
		groups[cid].push_back(idTrackPair.second);	// cid - 1?
	}

	// For each cluster of trajectories, compute a super track representation
	for (size_t cid = 0; cid < groups.size(); ++cid) {
		if (groups[cid].size() > 0) {
			superTracks.addTrack(cid, computeSuperTrackFromGroup(groups[cid]));	// cid become superTrackId			
		}
	}

	std::cout << superTracks.size() << "non empty super tracks" << std::endl;
 	return superTracks;
}

// Reads the graph (computed in BuildGraph.cpp) defined on original tracks 
// where nodes are trajectories and distances are measured by a combination of spatial distances and 
// frame misalignment penalties (see Corso's eq 4)
Edges readDij(const std::string& filename, const int numPrimitiveTracks, float& dijSum) {
	Edges dij(numPrimitiveTracks, std::vector<float>(numPrimitiveTracks, -1));	// -1 to replace inf
	std::vector<std::string> lines;
	readFileIntoStrings(filename, lines);
	std::string::size_type sz;
	
	for (size_t i = 0; i < lines.size(); ++i) {
		std::vector<std::string> strs;
		boost::split(strs, lines[i], boost::is_any_of(" "));
		for (const auto& str : strs) {
			std::pair<int, float> id_and_distance = parseIntoPair(str);
			dijSum += id_and_distance.second;
			dij[i][id_and_distance.first] = id_and_distance.second;
			dij[id_and_distance.first][i] = id_and_distance.second;
		}
	}	
	return dij;
}

// Converts distances (between 0 to inf) to similarity values (between 0 to 1)
Edges distanceToSimilarityViaGaussianKernel(const Edges& dij, const float dijSum) {
	Edges sij(dij.size(), std::vector<float>(dij.size(), 0));

	float sigma = dijSum / (dij.size() * dij.size());
	// construct sij
	for (int i = 0; i < dij.size(); ++i) {
		for (int j = 0; j < dij.size(); ++j) {
			if (dij[i][j] > 0)
				sij[i][j] = std::exp(-dij[i][j] * dij[i][j] / sigma / sigma / 2); 
		} 
	}
	return sij;
}

Edges computeSimilarities(
	const std::string& filename,
	const int numPrimitiveTracks, 
    const std::unordered_map<int, int>& clusterId,
    const int numClusters) {
	
	float dijSum = 0.0;
	Edges dij = readDij(filename, numPrimitiveTracks, dijSum);

	Edges sij = distanceToSimilarityViaGaussianKernel(dij, dijSum);

	// similarities of supertracks
	Edges ret(numClusters, std::vector<float>(numClusters, 0));

	for (int i = 0; i < sij.size(); ++i) {
		for (int j = 0; j < sij.size(); ++j) {
			int ci = clusterId.find(i)->second;
			int cj = clusterId.find(j)->second;
			// Self similarities are allowed
			if (sij[i][j] > 0) {
				ret[ci][cj] += sij[i][j];
				if (ci != cj) {
					ret[cj][ci] += sij[i][j];
				}
			}
		}
	}

	return ret;
}

// similarities is C x C
void writeSimilaritiesToFile(const std::string& path, const Edges& similarities) {
	FILE* fout = fopen(path.c_str(), "w");
	for (const auto& v : similarities) {
		for (const auto& vv : v) {
			fprintf(fout, "%f ", vv);
		}
		fprintf(fout, "\n");
	}
}

// Prints values of a type of feature
template <typename Functor>
std::string printFeature(const track& t) {
	
	const std::vector<float>& values = Functor()(t);
	
	std::ostringstream oss;
	std::copy(values.begin(), values.end()-1,
        std::ostream_iterator<float>(oss, " "));			// float values are seperated by spaces
	oss << values.back();
	
	return oss.str();
}

void writeSuperTracksToFile(const std::string& path, const trackList& superTracks) {
	std::ofstream fout;
	fout.open(path, std::fstream::in | std::fstream::out | std::fstream::app);
	for (const auto& id_track : superTracks.tracks()) {
		// Print trackId/cid
		fout << id_track.first << " ";
		// Print features
		fout << printFeature<DisplacementsGetter>(id_track.second) << " ";
		fout << printFeature<HogGetter>(id_track.second) << " ";
		fout << printFeature<HofGetter>(id_track.second) << " ";
		fout << printFeature<MbhXGetter>(id_track.second) << " ";
		fout << printFeature<MbhYGetter>(id_track.second) << " ";
		fout << std::endl;
	}

	fout.close();
}
// Reads NxN and returns CxC
int main(int argc, char** argv) {

	// path to sortedTrajecotries and result.txt
	const std::string primitiveGraphPath = argv[1];	
	const std::string clusterResultPath = argv[2];	
	const int numClusters = std::stoi(argv[3]);
	const std::string videoCategory = argv[4];
	const std::string vid = argv[5];

	// Reads result.txt, sortedTrajectories.out
	// track id -> cid
	std::cout << "reading cluster assignments" << std::endl;
	std::unordered_map<int, int> clusterId = readClusterId(clusterResultPath);

	const std::string videoName = videoCategory + vid;
	trackList tList;
	restoreTrackList(primitiveGraphPath + videoName + "_sortedTrajectories.out", tList);	
	std::cout << tList.tracks().size() << " tracks are restored" << std::endl;
	// Merge trajectories in the same cluster, computes a representative "supertrack" for each cluster	
	// cid becomes superTrackId
	std::cout << "Computing super tracks" << std::endl;
	trackList superTracks =	computeSuperTracks(numClusters, clusterId, tList);								
	std::cout << "Number of supertracks " << superTracks.size() << std::endl;

	// Returns a CxC matrix.
	std::cout << "Computing similarities" << std::endl;
	Edges similarities = computeSimilarities(primitiveGraphPath + videoName + "_dij.txt", tList.tracks().size(), clusterId, superTracks.size());
	
	/* ARCHIVE OUTPUTS*/
	std::cout << "Output super tracks to archive" << std::endl;
	videoRep video(superTracks, ucfActionClassMap[videoCategory], std::stoi(vid), -1, -1);	// Set videoWidth and videoHeight to nil
  	std::ofstream ofs(clusterResultPath + "archive/" +videoCategory + "_" + vid + ".out");
	{
	    boost::archive::binary_oarchive oa(ofs);
	    oa << video;   	 							// archive and stream closed when destructors are called
	}

	/* RAW DATA OUTPUTS*/
	std::cout << "Writing similarities between super tracks" << std::endl;
	writeSimilaritiesToFile(clusterResultPath + "similarities/" + videoName + "_similarities.txt", similarities);

	std::cout << "Writing super tracks in txt form" << std::endl;
	writeSuperTracksToFile(clusterResultPath + "supertracks/" + videoName + "_superTracks.txt", superTracks);

	std::cout << "Writing coords" << std::endl;
	writeCoordsToFile(clusterResultPath + "superTrackCoords/" + videoName + "_coords.out", superTracks);
	return 0;
}
