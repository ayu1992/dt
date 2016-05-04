#include "BoostRelatedHelpers.h"
#include <boost/algorithm/string.hpp>

#include <fstream>
int main(int argc, char** argv) {
	std::string traingingSetPath = argv[1];
	std::string line;
	
	std::string relabledTrainingSetPath = argv[2];
	std::ofstream fout;
	fout.open (relabledTrainingSetPath, std::fstream::in | std::fstream::out | std::fstream::app);

	std::ifstream fin(traingingSetPath);
	if (fin) {
		while (getline(fin, line)) {
			// locate the label
			std::size_t location = line.find(" ");	// contents will start at location + 1
			std::string label_str = line.substr(0, location);
			int label = std::atoi(label_str.c_str());

			// change the label; or not
			int relabel = label;
			switch (label) {
				case 2:
				case 12:
					relabel = 1; break;	
				case 4:
					relabel = 3; break;
				default: 
					break;
			}

			// write it out
			fout << relabel;
			fout << line.substr(location, line.length());
			fout << std::endl;
		}

		fin.close();
	}

	fout.close();
	return 0;
}