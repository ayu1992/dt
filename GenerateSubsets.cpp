#include "BoostRelatedHelpers.h"
#include "BoostMatrixHelpers.h"

using namespace boost::numeric::ublas;
// input : a super track / track file
// distance: edges (5 channel) / euclidean
// output: subsets (set index; scale)
// later: plot subsets in different color
// later: make this an interface, implements random walk strategy
const float epsilon = 0.000001;

// 	Define a uniform dist. of S values
std::vector<float> thresholds = {0.04, 0.035, 0.03, 0.025, 0.02};

matrix<float> readEdges(const std::string& edgesFile, const int N) {
	
	matrix<float> ret = zero_matrix<float>(N, N);
	std::string line;

	std::ifstream fin(edgesFile.c_str());

	for (int row = 0; row < N; ++row) {
		std::getline(fin, line);

		std::vector<std::string> tokens;
		boost::split(tokens, line, boost::is_any_of(" \n"));
		
		for (int j = 0; j < N; ++j) {
			ret(row, j) = std::stof(tokens[j]);
		}
	}

	fin.close();
	return ret;
}

matrix<float> getWeightedDegrees(const matrix<float>& A) {
	int N = A.size1();		// A, D are NxN matrices

	matrix<float> D = zero_matrix<float>(N, N);
	
	for (int i = 0; i < N; ++i) {
		float weightedDegree = 0.0;
		for (int j = 0; j < N; ++j) {
			weightedDegree += A(i,j);
		}
		D(i,i) = weightedDegree;
	}
	return D;
}

/**
 * Format: 
 * Threshold\n
 * a line for each set
*/
void writeSubsetsToOutput(
	const std::string& outputName, 
	const std::unordered_map<float, std::set<std::vector<int>>>& thresholdToSubsets) {
	std::ofstream fout;
	fout.open (outputName, std::fstream::in | std::fstream::out | std::fstream::app);
	for (const auto& threshToSubsetsPair : thresholdToSubsets) {
		fout << threshToSubsetsPair.first << std::endl;
		for (const auto& v : threshToSubsetsPair.second) {
			printVector(v, fout);
		}
	}
	fout.close();
}

void printWeightedDegrees(const matrix<float>& Dinv) {
	std::map<float, int> highestWeightedDegrees;
	for (int i = 0; i < Dinv.size1(); ++i) {
		highestWeightedDegrees.emplace(Dinv(i,i), i);
	}

	std::cout << "Print indices in order of highest weigthed degrees" << std::endl;
	for (const auto& keyValPair : highestWeightedDegrees) {
		std::cout << keyValPair.second << " ";
	}
	std::cout << std::endl;
}

matrix<float> parseDij(const std::string& filename, const int N) {
	matrix<float> ret = zero_matrix<float>(N, N);
	
	std::vector<std::string> lines;
	readFileIntoStrings(filename, lines);
	std::string::size_type sz;
	const size_t num_lines = lines.size();
	
	for (size_t i = 0; i < num_lines; ++i) {
		std::vector<std::string> strs;
		boost::split(strs, lines[i], boost::is_any_of(" "));
		for (const auto& str : strs) {
			std::pair<int, float> id_and_distance = parseIntoPair(str);
			int j = id_and_distance.first;
			ret(i, j) += id_and_distance.second;
			if (i != j) ret(j, i) += id_and_distance.second;
		}
	}
	return ret;
}

int main(int argc, char** argv) {
	const std::string supertrackFile = argv[1]; //home/hydralisk/Documents/dt/sJHMDB/Training/ClusteredTrajectories/sample=10000/r=0.05/c=100/archive/catch_1.out
	const std::string edgesFile = argv[2];	///home/hydralisk/Documents/dt/sJHMDB/Training/ClusteredTrajectories/sample=10000/r=0.05/c=100/edges/		

	trackList tList;
	restoreTrackList(supertrackFile, tList);

	std::vector<std::pair<int, track>> tracks = tList.tracks();
	int N = tracks.size();

	std::cout << N <<  "tracks" << std::endl;

	std::cout << "Read adjacency matrix A" << std::endl;
	matrix<float> A = parseDij(edgesFile, N);

	std::cout << A.size1() << "rows and " << A.size2() << " columns" << std::endl;

	// Symmetric check
	if (A.size1() != A.size2()) 	return -1;
	
	// Remove self edges
	for (int i = 0; i < A.size1(); ++i) {
		A(i,i) = 0.0;
	}

	std::cout << "Calculate degrees D" << std::endl;
	matrix<float> D = getWeightedDegrees(A);

	std::cout << "Calculate D inverse" << std::endl;
	matrix<float> Dinv = zero_matrix<float>(N, N);
	InvertMatrix<float>(D, Dinv);

	printWeightedDegrees(Dinv);

	std::cout << "Calculate transitional probability matrix W" << std::endl;
	matrix<float> W = prod(A, Dinv);
	//printMatrix(W);
	//  Loop till W^t+1 - W^t < epsilon
	int iter = 0;
	matrix<float> W_this(W);
	matrix<float> W_next = prod(W, W);			
	float error;
	while ((error = l2Norm(W_next, W_this)) > epsilon) {
		W_this = W_next;
		W_next = prod(W_this, W);	
		std::cout << "iteration : " << iter << " error: " << error << std::endl;
		++iter;
	}
	
	W = W_next;

	std::vector<int> aSubset;
	std::set<std::vector<int>> subsets;

	//  generate subsets under different thresholds
	std::unordered_map<float, std::set<std::vector<int>>> thresholdToSubsets;
	for (const auto& threshold : thresholds) {
		std::cout << "threshold: " << threshold << std::endl;
		subsets.clear();
		for (int col = 0; col < W.size1(); ++col) {
			aSubset.clear();
			// Linear search for all indices above threshold probability
			for (int row = 0; row < W.size2(); ++row) {
				// self will be included in the set and set will be sorted automatically
				if (W(row, col) >= threshold || col == row)	aSubset.push_back(row);
			}
			if(aSubset.size() > 1) {
				subsets.emplace(aSubset);
				//printVector(aSubset);
			}
		}
		thresholdToSubsets.emplace(threshold, subsets);
	}

	//  sensible output for subsets
	const std::string outputLocation = argv[3];
	writeSubsetsToOutput(outputLocation + "thresholdToSubsets.txt", thresholdToSubsets);
	//  visualize subsets
	return 0;
}