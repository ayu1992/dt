#include "ParserHelpers.h"

int main(int argc, char** argv) {
	std::string resultsPath = argv[1];
	std::string videoName = argv[2];

	std::ofstream fout;
	fout.open(resultsPath + "actualNumClusters", std::fstream::in | std::fstream::out | std::fstream::app);
	fout << videoName << ":" <<  countNonEmptyClusters(resultsPath) << std::endl;
	fout.close();
	return 0;
}