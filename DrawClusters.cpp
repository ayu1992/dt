#include "cvRelatedHelpers.h"
//#define VISUALIZE
using namespace cv;

// File and function documentation
void parseAndDraw(
	const std::vector<std::string> trjInStrings,
	std::vector<Mat>& frames,
	const std::vector<Scalar>& clusterColors) {

	for (const auto& str : trjInStrings) {
		std::vector<float> vals = split(str, ' ');
		int cid = vals[0];
		int endingFrame = vals[1];
		std::vector<Point2f> coords;
		for (auto vals_it = vals.begin() + 2; vals_it != vals.begin() + COORDS_LENGTH; vals_it += 2) {
			coords.emplace_back(*vals_it, *(vals_it + 1));
		}
		for(size_t i = 0; i < coords.size(); i++) {
			int frame = endingFrame + i + 1 - coords.size();
			circle(frames[frame], coords[i], 2, clusterColors[cid], -1, 10, 0);
		}
	}
}

/**
 *	Replay the original video and draw tracks on top of it
 */
// Input: cid, tracks
// ClusteredTrajecotories/r=2/c=500/ UCFSports/original/Diving-Side/ 1 500
int main(int argc, char** argv) {

	std::string path = argv[1];	// location of coords file

	// Doesn't require trajectories to be sorted
	std::vector<std::string> trjInStrings;
	readFileIntoStrings(path + "granularUnnormalizedCoords.out", trjInStrings);

    std::string videoPath = argv[2];	// location of video
    std::cout << "Reading video" << std::endl;

    std::string videoClass = argv[3];	// for use of output only

	int vid = std::stoi(argv[4]);

    int numClusters = std::stoi(argv[5]);

    std::vector<Scalar> clusterColors(numClusters);
    for (auto& s : clusterColors) s = getRandomColor();

	// Open Video to read
	VideoCapture capture = openVideo(videoPath + std::to_string(vid) + ".vob");

	std::vector<Mat> frames = getFramesFromVideo(capture);

	//std::cout << "Reading bounding boxes" << std::endl;
 	//std::vector<Box> boxes = readBoundingBoxes(videoPath + std::to_string(vid) + ".txt");

    std::cout << "[DrawClusters] Writing videos" << std::endl;
	
    // Draw circles on the frames
	std::cout << "[DrawClusters] Drawing circles" << std::endl;
	parseAndDraw(trjInStrings, frames, clusterColors);

	std::string outputVideoName = path + "rawTracks_" + videoClass + "_" + std::to_string(vid);
	
	// Open video to write; OpenCV only seems to have steady support for .avi outputs
	createVideoFromImages(
		outputVideoName + ".avi", 
		capture.get(CV_CAP_PROP_FOURCC), 
		10,
		Size(capture.get(CV_CAP_PROP_FRAME_WIDTH), capture.get(CV_CAP_PROP_FRAME_HEIGHT)), frames);

	// Do a system call and convert '.avi' into '.mp4' so videos we drew can be displayed in htmls
	std::string convertToMp4Command = "avconv -i " + outputVideoName + ".avi"+ " -c:v libx264 -c:a  copy "+ outputVideoName + ".mp4";
	int result_dontcare = std::system(convertToMp4Command.c_str());

	#ifdef VISUALIZE
	for(const auto& f : frames) {
	    imshow( "DenseTrack", f);
	    cvWaitKey(200);
	}
	#endif

	capture.release();
	return 0;
}