/**
 * Helper functions around OpenCV
 */
#include "ParserHelpers.h"
#include "Descriptors.h"

const int COORDS_LENGTH = 32;

// Mirrors a bounding box wrapper object as in BoostRelatedHelpers
// But this can be linked with OpenCV libraries
struct Box {
  Point2f UpperLeft;
  float width;
  float height;
};

// Checks if a point lies within a bounding box
bool isInBox(const Point2f& p, const Box& box) {
  float diffx = p.x - box.UpperLeft.x;
  float diffy = p.y - box.UpperLeft.y;
  return (diffx >= 0 && diffx <= box.width) && (diffy >= 0 && diffy <= box.height);
}

// Reads and parses bounding boxes
std::vector<Box> readBoundingBoxes(const std::string& filepath) {
  // read the boxes frame by frame
  std::string line;
  std::ifstream fin(filepath.c_str());
  std::vector<Box> boxes;
  if (!fin) {
    std::cerr << "Unable to open file : " << filepath << std::endl;
    return boxes;
  } 
  while(std::getline(fin, line)) {
    std::vector<float> val = split(line, ' ');
    boxes.emplace_back(Box{Point2f(val[0], val[1]), val[2], val[3]});
  }
  return boxes;
}

// Unnormalizes displacements into original coordinates. 
// Saves result in original vector 'normalizedDisplacements'. 
// First argument 'normalizedDisplacements' contains displacements of coordinates
//   (pair-wise subtraction and normalization of coordinates, see Descriptors.h isValid())
void unnormalizePoints(
  std::vector<Point2f>& normalizedDisplacements, 
  const float trajectoryLength, 
  const float mean_x, 
  const float mean_y) {
  for (Point2f& p : normalizedDisplacements) p = p * trajectoryLength;
  // Infer the last point.
  normalizedDisplacements.emplace_back(mean_x, mean_y);
  for (size_t i = 0; i < normalizedDisplacements.size() - 1; ++i) {
    normalizedDisplacements.back() = 
        normalizedDisplacements.back() + 
            (normalizedDisplacements[i] * (static_cast<float>(i + 1) / normalizedDisplacements.size()));
  }
  for (int i = normalizedDisplacements.size() - 2; i >= 0; --i) normalizedDisplacements[i] = normalizedDisplacements[i + 1] - normalizedDisplacements[i];
}

// Returns a random color
Scalar getRandomColor(void) {
    int r = rand() % 255;
    //r = (r + 300) / 2;  // uncomment for lighter palette
    int g = rand() % 255;
    //g = (g + 300) / 2;
    int b = rand() % 255;
   // b = (b + 300) / 2;
    return Scalar(r,g,b);
}

// Writes images from 'frames' into a video file
void createVideoFromImages(
  const std::string& videoOut, 
  const int fourcc, 
  const double fps, 
  const Size frameSize, 
  const std::vector<Mat>& frames) {
  VideoWriter writer;
  writer.open(videoOut.c_str(), fourcc, fps, frameSize, true);
  if(!writer.isOpened()) {
    fprintf(stderr, "Cannot open writer\n");
    return;
  }
  // Play all the frames
  namedWindow("DenseTrack", 0);
  for(const auto& f : frames) {
    writer.write(f);
  }

  writer.release();
}

// Opens a video file and simple error handling
VideoCapture openVideo(const std::string& videoPath) {
  VideoCapture capture;
  capture.open(videoPath.c_str());

  if(!capture.isOpened()) {
    fprintf(stderr, "Could not initialize capturing..\n");
  }
 
  return capture;
}

// Extracts frames from a video
std::vector<Mat> getFramesFromVideo(VideoCapture& capture) {
  std::vector<Mat> frames;
  Mat temporary, grey;

  while(true) {   
    Mat image;
    capture >> temporary;

    if (temporary.empty()) {
      break;
    }

    image.create(temporary.size(), CV_8UC3);
    grey.create(temporary.size(), CV_8UC1);
    temporary.copyTo(image);
    cvtColor(image, grey, CV_BGR2GRAY);
    
    // error handling
    frames.push_back(image);
  }
  return frames;
}