#include "cvRelatedHelpers.h"
using namespace cv;

int main() {
	VideoCapture capture;
	std::string video = "_.mp4";
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
		frame.copyTo(image);
		cvtColor(image, grey, CV_BGR2GRAY);
		
		// error handling
		frames.push_back(image);
	}

	int idx = 0;
	for (const auto& f : frames) {
		imwrite("/home/hydralisk/Documents/screensaver/" + std::to_string(idx++) + ".jpg", f);
	}
	return 0;
}