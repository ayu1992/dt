#include "BoostRelatedHelpers.h"
#include <string>

bool checkSymmetric(const std::string& filename) {
	std::vector<std::string> lines;
	readFileIntoStrings(filename, lines);
	std::string::size_type sz;
	std::cout << "reading file" << std::endl;	
	std::vector<std::vector<float>> dij(lines.size(), std::vector<float>(lines.size(), 0.0));
	
	std::cout << "Parsing lines" << std::endl;
	std::cout << "Dimension : " << lines.size() << std::endl;
	for (size_t i = 0; i < lines.size(); ++i) {
		std::vector<std::string> strs;
		boost::split(strs, lines[i], boost::is_any_of(" "));
		int j = 0;
		for (const auto& str : strs) {
			//std::pair<int, float> id_and_distance = parseIntoPair(str);
			//dij[i][id_and_distance.first] = id_and_distance.second;
			//dij[id_and_distance.first][i] = id_and_distance.second;
			if ( j < lines.size())
				dij[i][j++] = std::stof(str);
		}
	}

	std::cout << "Print diagonals" << std::endl;
	for (size_t i = 0; i < dij.size(); ++i) {
		std::cout << dij[i][i] << std::endl;
 	}

	for (int i = 0; i < dij.size(); ++i) {
		for (int j = 0; j < dij.size(); ++j) {
			if (dij[i][j] != dij[j][i])
				return false;
		}
	}

 		return true;
}
int main(int argc, char** argv) {
	const std::string filename = argv[1];
	std::cout << std::boolalpha << checkSymmetric(filename) << std::endl;
}
