#include "DenseTrack.h"
#include "Descriptors.h"
#include "ParserHelpers.h"
#include <cmath>
using namespace cv;

void readTrajectoriesAndDraw(
	const std::string& filename, std::vector<Mat>& frames,
	 const std::unordered_map<int, int>& clusterId, 
	 const std::vector<Scalar>& colorMap) {
	// read the trajectories by line
	std::string line;
	std::ifstream fin(filename.c_str());
	if (!fin) {
		std::cerr << "Unable to open file : " << filename << std::endl;
		return;
	}
	int trjId, endingFrame, trajectory_length, mean_x, mean_y;
	float scale; 
	// For each cluster, have a unique color
	// For each frame, for each scale, keep a set of track points
	
	while(std::getline(fin, line)) {
		std::vector<Point2f> points;
		std::vector<float> val = split(line, ' ');
	
		trjId = val[0];
		endingFrame = val[1];
		scale = val[2];
		trajectory_length = val[3];
		/* We need the following fields to de-normalize points in order to plot them*/
		mean_x = val[4];
		mean_y = val[5];
		for(int i = 6; i < val.size(); i += 2) {
			points.push_back(Point2f(val[i], val[i+1]));
		}
		// Un-normalize points.
		for (Point2f& p : points) p *= trajectory_length;
		// Infer the last point.
		points.emplace_back(mean_x, mean_y);
		for (size_t i = 0; i < points.size() - 1; ++i) {
			points.back() += (points[i] * (static_cast<float>(i + 1) / points.size()));
		}
		for (int i = points.size() - 2; i >= 0; --i) points[i] = points[i + 1] - points[i];

		Point2f point0 = points[0];
		point0 *=  scale;

		// for each frame, draw circles f = 14 -> 0
		for(int i = 0; i < points.size(); i++) {
			Point2f resizedPoint = points[i];
			//TODO: assign a color for each cluster
			circle(frames[endingFrame + i + 1 - points.size()], resizedPoint, 2, colorMap[clusterId.find(trjId)->second], -1, 8, 0);
		}
	}
	return;
}
/**
 *	Replay the original video and draw clusters on top of it
 */
int main(int argc, char** argv) {
	// trajectory id -> cluster id
	std::unordered_map<int, int> clusterId;

	int numClusters;

	// trajectory id -> vector<Point2f>, scales, frames
	const std::string idFilename = "result.txt";
	const std::string trjFilename = "sortedTrajectories.txt";

	// Open Video
	//TODO: read video by filename
	VideoCapture capture;
	char* video = argv[1];

	std::istringstream iss(argv[2]);
    iss >> numClusters;
    std::cout << "Number of clusters: "<< numClusters << std::endl;

	capture.open(video);

	char* videoOut = argv[3];
	std::cout << videoOut << std::endl;
	VideoWriter writer;
	writer.open(videoOut,
				capture.get(CV_CAP_PROP_FOURCC), 
				//capture.get(CV_CAP_PROP_FPS),
                10,
                Size(capture.get(CV_CAP_PROP_FRAME_WIDTH), capture.get(CV_CAP_PROP_FRAME_HEIGHT)),
                true);

	if(!capture.isOpened()) {
		fprintf(stderr, "Could not initialize capturing..\n");
		return -1;
	}

	if(!writer.isOpened()) {
		fprintf(stderr, "Cannot open writer\n");
		return -1;
	}

	std::vector<Mat> frames;

	Mat frame, grey;
	while(true) {
		Mat image;
		capture >> frame;

		// error handling
		if(frame.empty())
			break;
		
		image.create(frame.size(), CV_8UC3);
		grey.create(frame.size(), CV_8UC1);
		frame.copyTo(image);
		cvtColor(image, grey, CV_BGR2GRAY);
		frames.push_back(image);
	}

	// Read cluster ids'
	readClusterId(idFilename, clusterId);
	
	// Create unique colors for each cluster
	std::vector<Scalar> colorMap;
	for(int i = 0; i < numClusters; i++) {
	    int r = rand() % 255;
	    r = (r + 100) / 2;
	    int g = rand() % 255;
   	    g = (g + 100) / 2;
	    int b = rand() % 255;
   	    b = (b + 100) / 2;
	    colorMap.push_back(Scalar(r, g, b));
	}

	// Read Trajectories and their points
	// Draw circles on the frames
	readTrajectoriesAndDraw(trjFilename, frames, clusterId, colorMap);

	// Play all the frames
	namedWindow("DenseTrack", 0);
	for(const auto& f : frames) {
		//imshow( "DenseTrack", f);
		//cvWaitKey(200);
		writer.write(f);
	}
	capture.release();
	writer.release();
	return 0;
}