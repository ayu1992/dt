#include "cvRelatedHelpers.h"
#define VISUALIZE
using namespace cv;

void parseAndDraw(
	const std::vector<std::string> trjInStrings,
	std::vector<Mat>& frames,
	const std::vector<Box>& boxes) {

	for (const auto& str : trjInStrings) {
		std::vector<float> vals = split(str, ' ');
		int endingFrame = vals[0];
		std::vector<Point2f> coords;
		for (auto vals_it = vals.begin() + 1; vals_it != vals.begin() + COORDS_LENGTH - 1; vals_it += 2) {
			coords.emplace_back(*vals_it, *(vals_it + 1));
		}

		for(int i = 0; i < coords.size(); i++) {
			// Draw circle
			int frame = endingFrame + i + 1 - coords.size();
			circle(frames[frame], coords[i], 2, getRandomColor(), -1, 10, 0);
			// Draw bounding box
			//Box box = boxes[frame];
			//Point2f upperleft = boxes[frame].UpperLeft;
			//Point2f upperright = upperleft + Point2f(box.width, 0);
			//Point2f lowerleft = upperleft + Point2f(0, box.height);
			//Point2f lowerright = lowerleft + Point2f(box.width, 0);
			//cv::line(frames[frame],upperleft, upperright, Scalar(0,255,0), 2 ,8, 0);
			//cv::line(frames[frame],upperleft, lowerleft, Scalar(255,255,255), 2, 8, 0);
			//cv::line(frames[frame],upperright, lowerright, Scalar(0,0,255), 2, 8, 0);
			//cv::line(frames[frame],lowerleft, lowerright, Scalar(255,0,0), 2, 8, 0);
		}
	}
}

/**
 *	Replay the original video and draw tracks on top of it
 */
// Input: tracks
// ClusteredTrajecotories/r=2/c=500/ UCFSports/original/Diving-Side/ 1
int main(int argc, char** argv) {

	std::string outPath = argv[1];	// location of coords file

	// Doesn't require trajectories to be sorted
	std::vector<std::string> trjInStrings;
	readFileIntoStrings(outPath + "UnnormalizedCoords.out", trjInStrings);

    std::string inpath = argv[2];	// location of video

	int vid;
	std::istringstream getVid(argv[3]);
    getVid >> vid;

	// Open Video to read
	VideoCapture capture;
	std::string video = inpath + std::to_string(vid) + ".avi";
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

	std::cout << "Reading bounding boxes" << std::endl;
 	std::vector<Box> boxes = readBoundingBoxes(inpath + std::to_string(vid) + ".txt");

    // Draw circles on the frames
	std::cout << "[DrawClusters] Drawing circles" << std::endl;
	std::cout << frames.size() << "Frames in total" << std::endl;
	parseAndDraw(trjInStrings, frames, boxes);

	std::cout << "[DrawClusters] Writing videos" << std::endl;
	// Open video to write
	createVideoFromImages(
		outPath + std::to_string(vid) + ".avi", 
		capture.get(CV_CAP_PROP_FOURCC), 
		10,
		Size(capture.get(CV_CAP_PROP_FRAME_WIDTH), capture.get(CV_CAP_PROP_FRAME_HEIGHT)), frames);

	#ifdef VISUALIZE
	for(const auto& f : frames) {
	    imshow( "DenseTrack", f);
	    cvWaitKey(200);
	}
	#endif

	capture.release();
	return 0;
}