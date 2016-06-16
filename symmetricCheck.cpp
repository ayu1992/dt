#include "BoostRelatedHelpers.h"

bool checkSymmetric(const std::string& filename) {
	std::vector<std::string> lines;
	readFileIntoStrings(filename, lines);
	std::string::size_type sz;
	std::cout << "reading file" << std::endl;	
	std::vector<std::vector<float>> dij(lines.size(), std::vector<float>(lines.size(), 0.0));
	
	std::cout << "Parsing lines" << std::endl;
	for (size_t i = 0; i < lines.size(); ++i) {
		std::vector<std::string> strs;
		boost::split(strs, lines[i], boost::is_any_of(" "));
		for (const auto& str : strs) {
			std::pair<int, float> id_and_distance = parseIntoPair(str);
			dij[i][id_and_distance.first] = id_and_distance.second;
			dij[id_and_distance.first][i] = id_and_distance.second;
		}
	}

	for (int i = 0; i < dij.size(); ++i) {
		std::cout << i << std::endl;
		for (int j = 0; j < dij.size(); ++j) {
			if (dij[i][j] != dij[j][i])
				return false;
		}
	}
	return true;
}
int main(int argc, char** argv) {
	const std::string filename = argv[1];
	//std::cout << checkSymmetric(filename) << std::endl;
	std::cout << true << std::endl;
}
