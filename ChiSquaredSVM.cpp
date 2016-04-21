#include "BoostRelatedHelpers.h"
#include <cstdlib>
#include <boost/algorithm/string.hpp>

const int NUM_CHANNELS = 5;
using Data = std::vector<std::vector<float>>;		// N x 4000

// Read TrainingSet.out (N x 20000), TestSet.out (M x 20000)
// Parse them into 
// 1. labels (N x 1) and (M x 1)
// 2. Displacements (N x 4000) and (M x 4000)
// 3. Hog (N x 4000) and (M x 4000) ...etc
// Calculate Chi Squared matrices NxN and MxN
// Output KernelTraining.out, KernelTest.out
// And run the "svm-train -s 0 -t 4 KernelTraining.out kernel.model" ... procedures
std::vector<float> parseLine(std::vector<std::string>::iterator& strs_it) {
	std::vector<float> ret;  // TODO: If you know what size ret would be, maybe call vector::reserve.
	std::string::size_type sz;
	for (int i = 0; i < 4000; ++i) {
		std::vector<std::string> tmp;	// tmp would have 2 elements
		boost::split(tmp, *strs_it, boost::is_any_of(":"));
		float value = std::stof(tmp[1], &sz);
		ret.push_back(value);
		tmp.clear();
		++strs_it;
	}
	return ret;
}

void parseDataSet(
	const std::string& filename, 
	Data& displacements, // N x 4000
	Data& hog, 
	Data& hof,
	Data& mbhx, 
	Data& mbhy, 
	std::vector<int>& labels) {

	std::vector<std::string> lines;
	readFileIntoStrings(filename, lines);
	std::string::size_type sz;

	for (const auto& line : lines) {
		std::vector<std::string> strs;
		boost::split(strs, line, boost::is_any_of(" "));
		auto strs_it = strs.begin();
		// Get label
		int label = std::stoi(*strs_it, &sz);
		labels.push_back(label);
		++strs_it;

		displacements.push_back(parseLine(strs_it));
		hog.push_back(parseLine(strs_it));
		hof.push_back(parseLine(strs_it));
		mbhx.push_back(parseLine(strs_it));
		mbhy.push_back(parseLine(strs_it));		
	}
}

float computeDotProduct(const std::vector<float>& a, const std::vector<float>& b) {
	float dot_product = 0.0;
	auto it1 = a.begin();
	auto it2 = b.begin();
	for (; it1 != a.end(); ++it1, ++it2) {
		dot_product += (*it1 * *it2);
	}
	return dot_product;
}

// a, b are the same channel of two different samples. Size of a , b : 4000
float computeChiSquareDistance(const std::vector<float>& a, const std::vector<float>& b) {
	float chi_square = 0.0;
	auto it1 = a.begin();
	auto it2 = b.begin();
	for (; it1 != a.end(); ++it1, ++it2) {
		if (*it1 == 0 && *it2 == 0) continue;
		chi_square += ((*it1 - *it2) * (*it1 - *it2) / (*it1 + *it2));
	}
	return chi_square / 2;			// returns Dc
}

Data computeChiSquaredMatrix(const Data& testing, const Data& training, float& sum) {
	const int M = testing.size();
	const int N = training.size();
	// TODO: maybe assign sum=0.0 here.

	Data ret(M, std::vector<float>(N));
	for (int i = 0; i < M; ++i) {
		for (int j = 0; j < N; ++j) {
			ret[i][j] = computeChiSquareDistance(testing[i], training[j]);
			sum += ret[i][j];
		}
	}
	return ret;
}
// Initializes Ac for every channel
Data computeNormalizedChiSquareDistanceForTraining(	// Computes NxN
	const Data& training, 	// N x 4000
	float& Ac) {	
	const int N = training.size();

	float sum = 0.0;
	Data ret = computeChiSquaredMatrix(training, training, sum);
	
	Ac = sum / (N * N);

	for (auto& v : ret) {
		for (float& f : v) f /= Ac;
	}

	return ret;
}

// Uses Ac
Data computeNormalizedChiSquareDistanceForTesting(
	const Data& testing, 
	const Data& training, 
	const float Ac) {
	const int M = testing.size();
	const int N = training.size();

	float dummy_sum = 0.0;
	Data ret = computeChiSquaredMatrix(testing, training, dummy_sum);

	///float Ac_test = dummy_sum / (M * N);
	for (auto& v : ret) {
		for (float& f : v) f /= Ac;
	}

	return ret;
}	


Data buildKernelFromTrainingSet(
	const std::string& filename, 
	const float gamma,
	Data& displacements, 
	Data& hog, 
	Data& hof, 
	Data& mbhx, 
	Data& mbhy, 
	std::vector<int>& labels, 
	std::vector<float>& Ac) {

	// Fill up the arrays
	parseDataSet(filename, displacements, hog, hof, mbhx, mbhy, labels);
	std::cout << "Parsing Complete" << std::endl;

	const int N = displacements.size();	// N : number of training videos

	Data K(N, std::vector<float>(N, 0.0));

	auto add_channel_to_chi_squares = [&K, &Ac, N](const Data& channel, const int channel_num) {
		Data channel_chi_sq = computeNormalizedChiSquareDistanceForTraining(channel, Ac[channel_num]);
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) K[i][j] += channel_chi_sq[i][j];
		}
	};

	//add_channel_to_chi_squares(displacements, 0);
	add_channel_to_chi_squares(hog, 1);
	//add_channel_to_chi_squares(hof, 2);
	//add_channel_to_chi_squares(mbhx, 3);
	//add_channel_to_chi_squares(mbhy, 4);

	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j) K[i][j] = std::exp(-gamma * K[i][j]);
		//for (int j = 0; j < N; ++j) K[i][j] = std::exp(-1 * K[i][j]);
	}
	std::cout << "Kernel calculation complete" << std::endl;
	
	return K;
}

// displacements, hog ... are from training set
Data buildKernelForTestSet(
	const std::string& filename,
	const float gamma, 
	const Data& train_displacements, 
	const Data& train_hog, 
	const Data& train_hof,
	const Data& train_mbhx, 
	const Data& train_mbhy,
	std::vector<int>& test_labels, 
	const std::vector<float>& Ac) {

	Data test_displacements, test_hog, test_hof, test_mbhx, test_mbhy;

	// Fill up the arrays
	parseDataSet(filename, test_displacements, test_hog, test_hof, test_mbhx, test_mbhy, test_labels);
	std::cout << "Parsing test set Complete" << std::endl;

	const int M = test_displacements.size();	// N : number of training videos
	const int N = train_displacements.size();

	//std::vector<float> Ac(NUM_CHANNELS, 0.0);
	Data K(M, std::vector<float>(N, 0.0));

	auto add_channel_to_chi_squares = [&K, M, N](const Data& train_channel, const Data& test_channel, const float& ac) {
		//float tmp = 0.0;
		Data channel_chi_sq = computeNormalizedChiSquareDistanceForTesting(train_channel, test_channel, ac);
		for (int i = 0; i < M; ++i) {
			for (int j = 0; j < N; ++j) K[i][j] += channel_chi_sq[i][j];
		}
	};

	add_channel_to_chi_squares(train_displacements, test_displacements, Ac[0]);
	add_channel_to_chi_squares(train_hog, test_hog, Ac[1]);
	add_channel_to_chi_squares(train_hof, test_hof, Ac[2]);
	add_channel_to_chi_squares(train_mbhx, test_mbhx, Ac[3]);
	add_channel_to_chi_squares(train_mbhy, test_mbhy, Ac[4]);

	for (int i = 0; i < M; ++i) {
		for (int j = 0; j < N; ++j) K[i][j] = std::exp(-gamma * K[i][j]);
		//for (int j = 0; j < N; ++j) K[i][j] = std::exp(-K[i][j]);
	}
	std::cout << "Test Kernel calculation complete" << std::endl;
	
	return K;
}

void writeKernel(const std::string& filepath, const Data& K, const int M, const int N, const std::vector<int>& labels) {
	// Write Training Kernel
	std::ofstream fout;
	fout.open (filepath, std::fstream::in | std::fstream::out | std::fstream::app);
	std::cout << "M : " << M << std::endl;
	std::cout << "labels size : " << labels.size() << std::endl;
	for (int i = 0; i < M; ++i) {
		fout << labels[i] << " ";		// label
		fout << "0:" << (i + 1);
		for (int j = 0; j < N; ++j) fout << " " << (j + 1) << ":" << K[i][j];
		fout << std::endl;
	}

	fout.close();
}
int main(int argc, char** argv) {
	// I was trying to twist the Chi Squared form to : exp(- gamma * Dc)
	// Dc term : see http://www.di.ens.fr/willow/events/cvml2011/materials/CVML2011_Cordelia_bof.pdf page 34
	float gamma = static_cast<float>(std::atof(argv[1]));
	std::cout << gamma << std::endl;
	// Compute a chi-squared kernel over training set
	Data displacements, hog, hof, mbhx, mbhy;
	std::vector<int> train_labels, test_labels;

	std::vector<float> Ac_training(5, 0.0);

	Data train_K = buildKernelFromTrainingSet("NoClustering/Features/Displacements/TrainingSet.out", gamma, displacements, hog, hof, mbhx, mbhy, train_labels, Ac_training);	// N x N

	writeKernel("NoClustering/Features/HoG/KernelTraining.txt", train_K, train_K.size(), train_K.size(), train_labels);
	
	//Data test_K = buildKernelForTestSet("NoClustering/Features/Displacements/TestSet.out", gamma, displacements, hog, hof, mbhx, mbhy, test_labels, Ac_training);
	//writeKernel("NoClustering/Features/Displacements/KernelTest.txt", test_K, test_K.size(), test_K[0].size(), test_labels);

  return 0;
}