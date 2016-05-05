#include "cvRelatedHelpers.h"
//#define VISUALIZE
using namespace cv;

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
		for(int i = 0; i < coords.size(); i++) {
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

	int vid;
	std::istringstream getVid(argv[4]);
    getVid >> vid;

    int numClusters;
	std::istringstream getNumClusters(argv[5]);
    getNumClusters >> numClusters;

    std::vector<Scalar> clusterColors(numClusters);
    for (auto& s : clusterColors) s = getRandomColor();

	// Open Video to read
	VideoCapture capture;
	std::string video = videoPath + std::to_string(vid) + ".vob";
	std::cout << video << std::endl;
	capture.open(video.c_str());

	if(!capture.isOpened()) {
		fprintf(stderr, "Could not initialize capturing..\n");
		return -1;
	}

	std::vector<Mat> frames;

	Mat frame, grey;

	while(true) {		
		Mat image;
		capture >> frame;

		if (frame.empty()) {
			break;
		}

		image.create(frame.size(), CV_8UC3);
		grey.create(frame.size(), CV_8UC1);
		frame.copyTo(image);
		cvtColor(image, grey, CV_BGR2GRAY);
		
		// error handling
		frames.push_back(image);
	}

	//std::cout << "Reading bounding boxes" << std::endl;
 	//std::vector<Box> boxes = readBoundingBoxes(videoPath + std::to_string(vid) + ".txt");

    std::cout << "[DrawClusters] Writing videos" << std::endl;
	

    // Draw circles on the frames
	std::cout << "[DrawClusters] Drawing circles" << std::endl;
	std::cout << frames.size() << "Frames in total" << std::endl;
	parseAndDraw(trjInStrings, frames, clusterColors);

	std::string outputVideoName = path + "rawTracks_" + videoClass + "_" + std::to_string(vid);
	
	// Open video to write
	createVideoFromImages(
		outputVideoName + ".avi", 
		capture.get(CV_CAP_PROP_FOURCC), 
		10,
		Size(capture.get(CV_CAP_PROP_FRAME_WIDTH), capture.get(CV_CAP_PROP_FRAME_HEIGHT)), frames);

	std::string convertToMp4Command = "avconv -i " + outputVideoName + ".avi"+ " -c:v libx264 -c:a  copy "+ outputVideoName + ".mp4";
	std::system(convertToMp4Command.c_str());

	#ifdef VISUALIZE
	for(const auto& f : frames) {
	    imshow( "DenseTrack", f);
	    cvWaitKey(200);
	}
	#endif

	capture.release();
	return 0;
}