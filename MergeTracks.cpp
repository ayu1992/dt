#include <algorithm>
#include <cstdio>
#include <functional>
#include <string>
#include <sstream>
#include <utility>

#include <boost/algorithm/string.hpp>
#include "BoostRelatedHelpers.h"

using Edges = std::vector<std::vector<float>>;		// Express graph edges in terms of a KxK matrix

template <typename T>
void vectorAdd(const std::vector<T>& v1, std::vector<T>& v2) {
	std::transform(v1.begin(), v1.end(), v2.begin(), v2.begin(), std::plus<T>());
}

void addToTrack(const track& newTrack, track& targetTrack) {	// add newTrack to targetTracks
	vectorAdd(newTrack.displacements, targetTrack.displacements);
	vectorAdd(newTrack.coords, targetTrack.coords);
	vectorAdd(newTrack.hog, targetTrack.hog);
	vectorAdd(newTrack.hof, targetTrack.hof);
	vectorAdd(newTrack.mbhx, targetTrack.mbhx);
	vectorAdd(newTrack.mbhy, targetTrack.mbhy);
}

struct TakeAverageForFeature {
  const float numTracksAdded;
  TakeAverageForFeature(int numTracksAdded) : numTracksAdded(numTracksAdded) {}

  template <typename T>
  void operator()(std::vector<T>& v) const {
  	std::transform(v.begin(), v.end(), v.begin(), [this](const T& e){ return e / numTracksAdded; });
  }
};

// find supertrack representation for a group of tracks
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

	return superTrack;
}

// clusterId : trajId -> cid
// tList is sorted in endingFrame by ascending order
trackList computeSuperTracks(
	const int numClusters, 
	const std::unordered_map<int, int>& clusterId, 
	const trackList& tList) {
	
	trackList superTracks;			// rvo
	
	// place the trajectories to their respective groups
	std::vector<std::vector<track>> groups(numClusters, std::vector<track>());
	for (const auto& idTrackPair : tList.tracks()) {	// pair<track id, track>
		int cid = clusterId.find(idTrackPair.first)->second;	
		groups[cid].push_back(idTrackPair.second);	// cid - 1?
	}

	// For each group of trajectories, compute a representation
	for (int cid = 0; cid < groups.size(); ++cid) {
		if (groups[cid].size()) {
			superTracks.addTrack(cid, computeSuperTrackFromGroup(groups[cid]));	// cid become superTrackId			
		}
	}

 	return superTracks;
}

// Parse a string like "3:5.5" into a pair<int, float>.
std::pair<int, float> parseIntoPair(const std::string& s) {
	std::vector<std::string> tmp;	// tmp would have 2 elements
	boost::split(tmp, s, boost::is_any_of(":"));

	if (tmp.size() != 2) {
		std::cout << "Failed to parse pair: " << s << std::endl;
		return {-1, -1};
	}
	return {std::stoi(tmp[0]), std::stof(tmp[1])};
}

Edges computeEdgeWeights(
	const std::string& filename,
    const std::unordered_map<int, int>& clusterId,
    int C) {

	Edges ret(C, std::vector<float>(C, 0));
	
	std::vector<std::string> lines;
	readFileIntoStrings(filename, lines);
	std::string::size_type sz;
	const size_t num_lines = lines.size();
	
	for (size_t i = 0; i < num_lines; ++i) {
		std::vector<std::string> strs;
		boost::split(strs, lines[i], boost::is_any_of(" "));
		for (const auto& str : strs) {
			std::pair<int, float> id_and_distance = parseIntoPair(str);
			int ci = clusterId.find(i)->second;
			int cj = clusterId.find(id_and_distance.first)->second;
			//if (ci == cj) continue;
			ret[ci][cj] += id_and_distance.second;
			if (ci != cj) ret[cj][ci] += id_and_distance.second;
		}
	}
	return ret;
}

// edges is C x C
void writeEdgesToFile(const std::string& path, const Edges& edges) {
	FILE* fout = fopen(path.c_str(), "w");
	for (const auto& v : edges) {
		for (const auto& vv : v) {
			fprintf(fout, "%f ", vv);
		}
		fprintf(fout, "\n");
	}
}

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
		fout << "displacements: " << printFeature<DisplacementsGetter>(id_track.second);
		fout << "hog: " << printFeature<HogGetter>(id_track.second);
		fout << "hof: " << printFeature<HofGetter>(id_track.second);
		fout << "mbhx: " << printFeature<MbhXGetter>(id_track.second);
		fout << "mbhy: " << printFeature<MbhYGetter>(id_track.second);
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
	std::cout << "tracks are restored" << std::endl;
	// Merge trajectories in the same cluster, computes a representative "supertrack" for each cluster	
	// cid becomes superTrackId
	std::cout << "Computing super tracks" << std::endl;
	trackList superTracks =	computeSuperTracks(numClusters, clusterId, tList);								

	// Returns a CxC matrix.
	std::cout << "Computing edge weights" << std::endl;
	Edges edgeWeights = computeEdgeWeights(primitiveGraphPath + videoName + "_dij.txt", clusterId, numClusters);

	// Output to archive, print edge weights
	
	/* ARCHIVE OUTPUTS*/
	std::cout << "Output super tracks to archive" << std::endl;
	videoRep video(superTracks, ucfActionClassMap[videoCategory], std::stoi(vid), -1, -1);	// Set videoWidth and videoHeight to nil
  	std::ofstream ofs(clusterResultPath + videoCategory + "_" + vid + ".out");
	{
	    boost::archive::binary_oarchive oa(ofs);
	    oa << video;   	 							// archive and stream closed when destructors are called
	}

	/* RAW DATA OUTPUTS*/
	std::cout << "Writing edge weights" << std::endl;
	writeEdgesToFile(clusterResultPath + videoName + "_edges.txt", edgeWeights);

	std::cout << "Writing super tracks in txt form" << std::endl;
	writeSuperTracksToFile(clusterResultPath + videoName + "_superTracks.txt", superTracks);

	//std::cout << "Writing coords" << std::endl;
	//writeCoordsToFile(clusterResultPath, superTracks);
	return 0;
}
