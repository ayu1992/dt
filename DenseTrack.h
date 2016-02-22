#ifndef DENSETRACK_H_
#define DENSETRACK_H_

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>
#include <ctype.h>
#include <unistd.h>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <list>
#include <string>

using namespace cv;

int start_frame = 0;
int end_frame = INT_MAX;
int scale_num = 8;        // Max pyramid height
const float scale_stride = sqrt(2);

// parameters for descriptors
int patch_size = 32;      // frame dimension (must) > patch_size
int nxy_cell = 2;
int nt_cell = 3;
float epsilon = 0.05;
const float min_flow = 0.4;

// parameters for tracking
double quality = 0.001;
int min_distance = 5;
int init_gap = 1;
int track_length = 15;

// parameters for rejecting trajectory
const float min_var = sqrt(3);
const float max_var = 50;
const float max_dis = 20;

struct RectInfo {
	int x;       // top left corner
	int y;
	int width;
	int height;
};

struct SeqInfo {
    int width;   // resolution of the video
    int height;
    int length;  // number of frames
};

struct TrackInfo {
    int length;  // length of the trajectory
    int gap;     // initialization gap for feature re-sampling 
};

struct DescInfo {
    int nBins;   // number of bins for vector quantization
    bool isHof;  // Because it requires 9 bins instead of 8 
    int nxCells; // number of cells in x direction
    int nyCells; 
    int ntCells;
    int dim;     // dimension of the descriptor
    int height;  // size of the block for computing the descriptor
    int width;
};    // a Desc == a cell? 

// integral histogram for the descriptors
struct DescMat {
    int height;
    int width;
    int nBins;
    float* desc;
};

// Information about a trajectory
struct TrackTubeInfo {  
    int endingFrame;  // the last frame of the trajectory
    float mean_x;  // mean value of the coords
    float mean_y;
    float var_x;   // var of the coords
    float var_y;  
    float length;    // sum of Eucledian distance of points on the trajectory
    float scale;   // The trajectory is computed on which scale
    float x_pos;
    float y_pos;
    float t_pos;
};

class Track      // a Track is a list of points and lists of descriptors
{
public:
    std::vector<Point2f> point;
    std::vector<float> hog;
    std::vector<float> hof;
    std::vector<float> mbhX;
    std::vector<float> mbhY;
    int index;  // what is index? : size of points

    Track(
        const std::vector<Point2f>& points, 
        const std::vector<float>& hog, 
        const std::vector<float>& hof, 
        const std::vector<float>& mbhX, 
        const std::vector<float>& mbhY)
     : point(points), hog(hog), hof(hof), mbhX(mbhX), mbhY(mbhY) {}

    Track(
        const Point2f& point_, 
        const TrackInfo& trackInfo,
        const DescInfo& hogInfo,
        const DescInfo& hofInfo,
        const DescInfo& mbhInfo)
        : point(trackInfo.length+1),
          hog(hogInfo.dim*trackInfo.length),
          hof(hofInfo.dim*trackInfo.length), 
          mbhX(mbhInfo.dim*trackInfo.length), 
          mbhY(mbhInfo.dim*trackInfo.length) {
        index = 0;
        point[0] = point_;
    }

    void addPoint(const Point2f& point_) {
        index++;
        point[index] = point_;
    }
};

struct TrackTube {
    TrackTubeInfo trackTubeInfo;
    Track track;
};
#endif /*DENSETRACK_H_*/
