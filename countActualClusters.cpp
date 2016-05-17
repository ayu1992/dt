#include "ParserHelpers.h"

/**
 * This binary is executed right after pspectralclustering
 * It does a simple task of counting the acutal number of non-empty clusters for a particular video
 * and writing the count to "$PROJECT_PATH/actualNumClusters"
 */
int main(int argc, char** argv) {
	std::string resultsPath = argv[1];
	std::string videoName = argv[2];

	std::ofstream fout;
	fout.open(resultsPath + "actualNumClusters", std::fstream::in | std::fstream::out | std::fstream::app);
	fout << videoName << ":" <<  countNonEmptyClusters(resultsPath) << std::endl;
	fout.close();
	return 0;
}