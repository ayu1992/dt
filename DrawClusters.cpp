#include "DenseTrack.h"
#include "Descriptors.h"
#include "ParserHelpers.h"
#include "CVUtils.h"
using namespace cv;

void parseTrajectoriesAndDraw(
	const std::vector<std::string>& trajInStrings,
	std::vector<Mat>& frames,
	const std::unordered_map<int, int>& clusterId, 
	const Scalar& circleColor,
	const int largestClusterId,
	const std::vector<Box>& boxes) {

	int trjId, endingFrame;
	float scale, trajectory_length, mean_x, mean_y; 
	
	for (auto const& track : trajInStrings) {
	// For each frame, for each scale, keep a set of track points

		std::vector<Point2f> points;
		parseLineFromSortedTrajectories(track, trjId, endingFrame, scale, trajectory_length, mean_x, mean_y, points);
		
		if (clusterId.find(trjId)->second != largestClusterId) {
			continue;
		}

		unnormalizePoints(points, trajectory_length, mean_x, mean_y);

		// Un-normalize points and plot
		for(int i = 0; i < points.size(); i++) {
			// Draw circle
			int frame = endingFrame + i + 1 - points.size();
			circle(frames[frame], points[i], 2, circleColor, -1, 3, 0);
			// Draw bounding box
			
	//		std::cout << " boxes has " << boxes.size() << " frames, accessing frame " << frame << std::endl;
			Box box = boxes[frame];

	//		std::cout << "box Point : " << boxes[frame].UpperLeft.x << "," << boxes[frame].UpperLeft.y << " width : " << boxes[frame].width << std::endl;
	//		std::cout << "Resized point : " << resizedPoint.x << "," << resizedPoint.y << std::endl;
	
			Point2f upperleft = boxes[frame].UpperLeft;
			Point2f upperright = upperleft + Point2f(box.width, 0);
			Point2f lowerleft = upperleft + Point2f(0, box.height);
			Point2f lowerright = lowerleft + Point2f(box.width, 0);

			cv::line(frames[frame],upperleft, upperright, Scalar(0,255,0), 2 ,8, 0);
			cv::line(frames[frame],upperleft, lowerleft, Scalar(255,255,255), 2, 8, 0);
			cv::line(frames[frame],upperright, lowerright, Scalar(0,0,255), 2, 8, 0);
			cv::line(frames[frame],lowerleft, lowerright, Scalar(255,0,0), 2, 8, 0);
	//		std::cout << "line drawn" << std::endl;
		}
	}
}

/**
 *	Replay the original video and draw clusters on top of it
 */
 // ClusteredTrajecotories/r=50/c=2/ InputVideos/Diving/ 22 2 
int main(int argc, char** argv) {

	int numClusters, vid;
	std::string outPath = argv[1];
	const std::string idFilename = outPath + "result.txt";
	const std::string trjFilename = outPath + "sortedTrajectories.out";

	std::istringstream getVid(argv[3]);
    getVid >> vid;

    std::istringstream getNumClusters(argv[4]);
    getNumClusters >> numClusters;

    std::string inpath = argv[2];

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

	// trajectory id -> cluster id
	std::unordered_map<int, int> clusterId;
	std::vector<int> clusterSizes(numClusters, 0);

	int largestClusterId = returnIdOfLargestCluster(idFilename, clusterId, clusterSizes);
	std::cout << "[DrawClusters] Dominant cluster contains "<< clusterSizes[largestClusterId] << "tracks" << std::endl;
 	
	Scalar color = getRandomColor();

	std::cout << "Reading bounding boxes" << std::endl;
 	std::vector<Box> boxes = readBoundingBoxes(inpath + std::to_string(vid) + ".txt");

	// Read Trajectories and their points
	std::vector<std::string> trajInStrings;

    std::cout << "Reading trajectories" << std::endl;
    readFileIntoStrings(trjFilename, trajInStrings);
    // Draw circles on the frames
	std::cout << "[DrawClusters] Drawing circles" << std::endl;
	parseTrajectoriesAndDraw(trajInStrings, frames, clusterId, color, largestClusterId, boxes);

	std::cout << "[DrawClusters] Writing videos" << std::endl;
	// Open video to write
	std::string videoOut = outPath + std::to_string(vid) + ".avi";
	VideoWriter writer;
	writer.open(videoOut.c_str(),
				capture.get(CV_CAP_PROP_FOURCC), 
				//capture.get(CV_CAP_PROP_FPS),
                10,
                Size(capture.get(CV_CAP_PROP_FRAME_WIDTH), capture.get(CV_CAP_PROP_FRAME_HEIGHT)),
                true);
	if(!writer.isOpened()) {
		fprintf(stderr, "Cannot open writer\n");
		return -1;
	}
	// Play all the frames
	namedWindow("DenseTrack", 0);
	for(const auto& f : frames) {
		imshow( "DenseTrack", f);
		cvWaitKey(200);
		writer.write(f);
	}
	capture.release();
	writer.release();
	return 0;
}