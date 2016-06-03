#include "BoostRelatedHelpers.h"
#include "BoostMatrixHelpers.h"

using namespace boost::numeric::ublas;
// input : a super track / track file
// distance: edges (5 channel) / euclidean
// output: subsets (set index; scale)
// later: plot subsets in different color
// later: make this an interface, implements random walk strategy
const float epsilon = 0.000001;
float MAX = std::numeric_limits<float>::max();

// 	Define a uniform dist. of S values
std::vector<float> thresholds = {0.001, 0.002, 0.005, 0.02, 0.03};

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

inline float gaussianKernel(const float distance, const float sigma) {
	return (distance == 0.0) ? MAX : std::exp(- std::pow(distance, 2) / std::pow(sigma, 2));
}

matrix<float> distanceToSimilarity(const matrix<float>& A_dis) {
	matrix<float> A_sim = zero_matrix<float>(A_dis.size1(), A_dis.size2());
	float mean = 0.0;
	for (int i = 0; i < A_dis.size1(); ++i) {
		for (int j = 0; j < A_dis.size2(); ++j) {
			mean += A_dis(i,j);
		}
	}
	
	mean /= (A_dis.size1() * A_dis.size2());
	float max = 0.0;
	for (int i =0; i < A_sim.size1(); ++i) {
		for (int j = 0; j < A_sim.size2(); ++j) {
			float gaussianDistance = gaussianKernel(A_dis(i, j), mean);
			max = (gaussianDistance > max) ? gaussianDistance : max;
			A_sim(i, j) =  gaussianDistance;
		}
	}

	for (int i =0; i < A_sim.size1(); ++i) {
		for (int j = 0; j < A_sim.size2(); ++j) {
			A_sim(i, j) = max - A_sim(i, j);
		}
	}

	return A_sim;
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
	std::multimap<float, int> highestWeightedDegrees;
	for (int i = 0; i < Dinv.size1(); ++i) {
		highestWeightedDegrees.emplace(Dinv(i,i), i);
	}

	std::cout << "Print indices in order of highest weigted degrees" << std::endl;
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

	/* For raw tracks and dij */
	/*trackList tList;
	restoreTrackList(supertrackFile, tList);
	std::vector<std::pair<int, track>> tracks = tList.tracks();
	int N = tracks.size();

	std::cout << N <<  "tracks" << std::endl;

	std::cout << "Read adjacency matrix A" << std::endl;
	matrix<float> A_dis = parseDij(edgesFile, N);
*/
	
/*
	videoRep video;
	restoreVideoRep(supertrackFile, video);

	std::vector<std::pair<int, track>> tracks = video.getTrackList().tracks();
	int N = tracks.size();

	std::cout << N <<  "tracks" << std::endl;
*/
	std::cout << "Read adjacency matrix A" << std::endl;
	matrix<float> A_dis = readEdges(edgesFile, N);

	std::cout << A_dis.size1() << "rows and " << A_dis.size2() << " columns" << std::endl;

	// Symmetric check
	if (A_dis.size1() != A_dis.size2()) 	return -1;
	
	// Remove self edges
	for (int i = 0; i < A_dis.size1(); ++i) {
		A_dis(i,i) = 0.0;
	}

	matrix<float> A = distanceToSimilarity(A_dis);

	std::cout << "Calculate degrees D" << std::endl;
	matrix<float> D = getWeightedDegrees(A);

	std::cout << "Calculate D inverse" << std::endl;
	matrix<float> Dinv = zero_matrix<float>(N, N);
	InvertDiagonalMatrix<float>(D, Dinv);

	printWeightedDegrees(Dinv);

	std::cout << "Calculate transitional probability matrix W" << std::endl;
	matrix<float> W = prod(A, Dinv);

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
	
	W = trans(W_next);	// rows sum to 1
	std::ofstream fout;
	fout.open ("test/W.txt", std::fstream::in | std::fstream::out | std::fstream::app);
	printMatrix(W, fout);

	float sum = 0.0;
/*
	for (int rowIdx = 0; rowIdx < W.size1(); ++rowIdx) {
		sum = 0.0;
		for (int colIdx = 0; colIdx < W.size2(); ++colIdx) {
			sum += W(rowIdx, colIdx);
		}
		std::cout << "row sum : " << sum << std::endl;
	}
*/
	// Each vector element is {(p1, n1), (p2, n2) ... }
	std::vector<std::multimap<float, int>> accProbsfromEachNode;
	std::multimap<float, int> distribution;
	std::multimap<float, int> accumulated;
	for (int rowIdx = 0; rowIdx < W.size1(); ++rowIdx) {
		// Construct the distribution for a node (as origin of a walk)
		distribution.clear();
		accumulated.clear();
		for (int colIdx = 0; colIdx < W.size2(); ++colIdx) {
			// distribution keeps (prob, node id) pairs
			// keys in std::map maintains sorted
			distribution.emplace(W(rowIdx, colIdx), colIdx);
		}

		// Accumulative probability
		float acc = 0.0;
		for (auto dist_it = distribution.begin(); dist_it != distribution.end(); ++dist_it){	
			acc += dist_it->first;
			accumulated.emplace(acc, dist_it->second);
		}
		accProbsfromEachNode.emplace_back(accumulated);
	}

	std::ofstream fcc;
	fcc.open ("test/acc.txt", std::fstream::in | std::fstream::out | std::fstream::app);
	for (int i = 0; i < accProbsfromEachNode.size(); ++i) {
		fcc << i << std::endl;
		for (const auto & keyValPair : accProbsfromEachNode[i]) {
			fcc << keyValPair.first << "," << keyValPair.second << " ";
		}
		fcc << std::endl;
	}

	std::vector<int> aSubset;
	std::set<std::vector<int>> subsets;

	//  generate subsets under different thresholds
	std::unordered_map<float, std::set<std::vector<int>>> thresholdToSubsets;

	for (const auto& threshold : thresholds) {
		std::cout << "threshold: " << threshold << std::endl;
		subsets.clear();
		for (int startNode = 0; startNode < accProbsfromEachNode.size(); ++startNode){
			aSubset.clear();
			for (const auto& keyValPair : accProbsfromEachNode[startNode]) {
				if (keyValPair.first > threshold) {
					break;
				}
				aSubset.push_back(keyValPair.second);
			}
			if (!aSubset.empty()) {
				aSubset.push_back(startNode);
				subsets.emplace(aSubset);				
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