#include "BoostRelatedHelpers.h"

int main() {
	std::string fpath = "NoClustering/Training/Riding-Horse_10.out";
	videoRep video;
	restoreVideoRep(fpath, video);
	std::cout << video.vid() << " " << video.videoWidth() << " " << video.videoHeight() << std::endl;

	// Get rid of trajectory indices
	for (const auto& pair : video.largestCluster().tracks()) {
		if (std::isnan(pair.first)) std::cout << "FUCK" << std::endl;
		// pair : int : track
		const track& t = pair.second;
		std::cout << t.endingFrame << " " << t.mean_x << " " << t.mean_y << " " << t.var_x << " " << t.var_y << " ";
		std::cout << t.length << " " << " " << t.scale << " " << t.x_pos << " " << t.y_pos << " " << t.t_pos << " ";
		std::for_each(t.coords.begin(), t.coords.end(), [](const point& p){ std::cout << p.x << " " << p.y << " ";});
		std::for_each(t.displacements.begin(), t.displacements.end(), [](const point& p){ std::cout << p.x << " " << p.y << " ";});
		std::for_each(t.hog.begin(), t.hog.end(), [](const float& f){ std::cout << f << " ";});
		std::for_each(t.hof.begin(), t.hof.end(), [](const float& f){ std::cout << f << " ";});
		std::for_each(t.mbhx.begin(), t.mbhx.end(), [](const float& f){ std::cout << f << " ";});
		std::for_each(t.mbhy.begin(), t.mbhy.end(), [](const float& f){ std::cout << f << " ";});
//		if (std::find(t.coords.begin(), t.coords.end(), NaN) != t.coords.end()) 	std::cout << "Coords" << std::endl;
//		if (std::find(t.displacements.begin(), t.displacements.end(), NaN) != t.displacements.end()) 	std::cout << "displacements" << std::endl;
		/*std::for_each(t.coords.begin(), t.coords.end(), 
	      [](const point& p) {
	        if (std::isnan(p.x) || std::isnan(p.y)) {
	          std::cout << "NAN detected!!! coords" << std::endl;
	        }
    	});

    	std::for_each(t.displacements.begin(), t.displacements.end(), 
	      [](const point& p) {
	        if (std::isnan(p.x) || std::isnan(p.y)) {
	          std::cout << "NAN detected!!!" << std::endl;
	        }
    	});

		std::for_each(t.hog.begin(), t.hog.end(), 
	      [](const float& f) {
	        if (std::isnan(f)) {
	          std::cout << "NAN detected hog!!!" << std::endl;
	        }
    	});

		std::for_each(t.hof.begin(), t.hof.end(), 
	      [](const float& f) {
	        if (std::isnan(f)) {
	          std::cout << "NAN detected hof!!!" << std::endl;
	        }
    	});

		std::for_each(t.mbhx.begin(), t.mbhx.end(), 
	      [](const float& f) {
	        if (std::isnan(f)) {
	          std::cout << "NAN detected mbhx!!!" << std::endl;
	        }
    	});

    	std::for_each(t.mbhy.begin(), t.mbhy.end(), 
	      [](const float& f) {
	        if (std::isnan(f)) {
	          std::cout << "NAN detected mbhy!!!" << std::endl;
	        }
    	});
    	*/


	}

}