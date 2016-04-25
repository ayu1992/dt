#include "ParserHelpers.h"
#include "Descriptors.h"

const int COORDS_LENGTH = 32;

struct Box {
  Point2f UpperLeft;
  float width;
  float height;
};

bool isInBox(const Point2f& p, const Box& box) {
  float diffx = p.x - box.UpperLeft.x;
  float diffy = p.y - box.UpperLeft.y;
  return (diffx >= 0 && diffx <= box.width) && (diffy >= 0 && diffy <= box.height);
}

std::vector<Box> readBoundingBoxes(std::string filepath) {
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

void unnormalizePoints(std::vector<Point2f>& points, const float trajectoryLength, const float mean_x, const float mean_y) {
  for (Point2f& p : points) p = p * trajectoryLength;
  // Infer the last point.
  points.emplace_back(mean_x, mean_y);
  for (size_t i = 0; i < points.size() - 1; ++i) {
    points.back() = points.back() + (points[i] * (static_cast<float>(i + 1) / points.size()));
  }
  for (int i = points.size() - 2; i >= 0; --i) points[i] = points[i + 1] - points[i];
}

Scalar getRandomColor(void) {
    int r = rand() % 255;
    //r = (r + 300) / 2;
    int g = rand() % 255;
    //g = (g + 300) / 2;
    int b = rand() % 255;
   // b = (b + 300) / 2;
    return Scalar(r,g,b);
}

void createVideoFromImages(const std::string& videoOut, const int fourcc, const double fps, const Size frameSize, const std::vector<Mat>& frames) {
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