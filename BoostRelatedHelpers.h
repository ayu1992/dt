#include "ParserHelpers.h"
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/assume_abstract.hpp>

struct point {
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & x;
    ar & y;
  }
  float x;
  float y;
  
  point() {};
  point(const float x, const float y): x(x), y(y) {};

  // multiplication that mutates self
  point& operator*=(float s) {
    x *= s;
    y *= s;
    return *this;
  }
};

point operator+(const point& p1, const point& p2)
{
  return point(p1.x + p2.x, p1.y + p2.y);
}

point operator-(const point& p1, const point& p2)
{
  return point(p1.x - p2.x, p1.y - p2.y);
}

// p *= 3
point operator*(const point& p1, const float s)
{
  return point(p1.x * s, p1.y * s);
  // return point(p1) *= s;
}

void unnormalizePoints(std::vector<point>& points, const float trajectoryLength, const float mean_x, const float mean_y) {
  for (point& p : points) p = p * trajectoryLength;
  // Infer the last point.
  points.emplace_back(mean_x, mean_y);
  for (size_t i = 0; i < points.size() - 1; ++i) {
    points.back() = points.back() + (points[i] * (static_cast<float>(i + 1) / points.size()));
  }
  for (int i = points.size() - 2; i >= 0; --i) points[i] = points[i + 1] - points[i];
  
  /*for (const auto& p : points) {
    if (std::isnan(p.x) || std::isnan(p.y)) std::cout << "ARRRRRHHHHH" << std::endl;
  }*/
}

struct track {
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & endingFrame & mean_x & mean_y & var_x & var_y;
    ar & length & scale & x_pos & y_pos & t_pos & displacements & coords;
    ar & hog & hof & mbhx & mbhy;
  }

  int endingFrame;
  float mean_x;
  float mean_y;
  float var_x;   // var of the coords
  float var_y;  
  float length;    // sum of Eucledian distance of Points on the trajectory
  float scale;   // The trajectory is computed on which scale
  float x_pos;
  float y_pos;
  float t_pos;
  std::vector<point> displacements;   // 15 x 2
  std::vector<point> coords;          // 16 x 2
  std::vector<float> hog;
  std::vector<float> hof;
  std::vector<float> mbhx;
  std::vector<float> mbhy;
  
  track() {};
  track(
    int endingFrame, float mean_x, float mean_y, float var_x, float var_y, float length, 
    float scale, float x_pos, float y_pos, float t_pos, std::vector<point> displacements, std::vector<point> coords, 
    std::vector<float> hog, std::vector<float> hof, std::vector<float> mbhx, 
    std::vector<float> mbhy) 
  : endingFrame(endingFrame), mean_x(mean_x), mean_y(mean_y), var_x(var_x), var_y(var_y), 
  length(length), scale(scale), x_pos(x_pos), y_pos(y_pos), t_pos(t_pos), displacements(displacements), coords(coords), 
  hog(hog), hof(hof), mbhx(mbhx), mbhy(mbhy) {}
};

struct NormalizedPointGetter {
  std::vector<float> operator()(const track& t) {
    std::vector<float> ret(2 * t.coords.size());
    for (size_t i = 0; i < t.coords.size(); ++i) {
      ret[2 * i] = t.coords[i].x;
      ret[2 * i + 1] = t.coords[i].y;
    }
    return ret;
  }
  static constexpr int dimension = 32;
};

struct HogGetter {
  const std::vector<float>& operator()(const track& t) { return t.hog; }
  static constexpr int dimension = 96;
};

struct HofGetter {
  const std::vector<float>& operator()(const track& t) { return t.hof; }
  static constexpr int dimension = 108;
};

struct MbhXGetter {
  const std::vector<float>& operator()(const track& t) { return t.mbhx; }
  static constexpr int dimension = 96;
};

struct MbhYGetter {
  const std::vector<float>& operator()(const track& t) { return t.mbhy; }
  static constexpr int dimension = 96;
};

class trackList {
private:
  friend class boost::serialization::access;
  friend std::ostream & operator<<(std::ostream &os, const trackList &tList);
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & _tracks;
  }

  std::vector<std::pair<int, track>> _tracks;

public: 
  trackList() {};
  trackList(const std::vector<std::pair<int, track>>& tracks): _tracks(tracks) {}
  
  /* TODO: track index should be handled by this class instead of caller*/
  void addTrack(int index, const track& t) {_tracks.emplace_back(index, t);}
  int size() const {return _tracks.size();}
  track getTrack(int index) const {return _tracks[index].second;}
  const std::vector<std::pair<int, track>>& tracks() const{ return _tracks;}
};

class videoRep {
private:
	friend class boost::serialization::access;
	friend std::ostream & operator<<(std::ostream &os, const videoRep& video);
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & _dominantCluster;
		ar & _classLabel;			// label of action class
    ar & _vid;
    ar & _videoWidth;
    ar & _videoHeight;
	}
	// a video is represented by One trajectory cluster/list
	trackList _dominantCluster;
	int _classLabel;	
	int _vid;
  int _videoWidth;
  int _videoHeight;

public:
	videoRep() {};
	videoRep(const trackList& dominantCluster, const int classLabel, const int vid, const int videoWidth, const int videoHeight): _dominantCluster(dominantCluster), _classLabel(classLabel), _vid(vid), _videoWidth(videoWidth), _videoHeight(videoHeight) {}

	const trackList& largestCluster() const {return _dominantCluster;}
	const int classLabel() const {return _classLabel;}
	const int vid() const {return _vid;}
  const int videoWidth() const {return _videoWidth;}
  const int videoHeight() const {return _videoHeight;}
};

/* Maybe templatize these two functions? */
void restoreTrackList(const std::string& path, trackList& tList) {
    std::ifstream ifs(path);    // contains A video
    boost::archive::binary_iarchive ia(ifs);
    ia >> tList;
}

void restoreVideoRep(const std::string& path, videoRep& video) {
    std::ifstream ifs(path);    // contains A video
    boost::archive::binary_iarchive ia(ifs);
    ia >> video;
}

struct Box {
  point UpperLeft;
  float width;
  float height;
};

bool isInBox(const point& p, const Box& box) {
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
    boxes.emplace_back(Box{point(val[0], val[1]), val[2], val[3]});
  }
  return boxes;
}

void parseFeaturesToTracks(
  const std::string& filename, std::vector<std::string>& trajInStrings, std::vector<track>& tracks, int& videoWidth, int& videoHeight) {
  
  readFileIntoStrings(filename, trajInStrings);
  
  for (auto const& str: trajInStrings) {
    std::vector<float> val = split(str, '\t');

    if (val[7] < 0.0000001) {
      std::cout << "length 0 detected" << std::endl;
      continue;   // a length 0 trajectory
    }
    // Substitue NaN with zeros

    videoWidth = static_cast<int>(val[0]);
    videoHeight = static_cast<int>(val[1]);

    std::vector<point> displacements;
    for (int i = TRACK_INFO_LEN; i < 2 * TRACK_LEN + TRACK_INFO_LEN - 1; i += 2) {
      displacements.push_back(point(val[i], val[i+1]));
    }

    std::vector<point> coords(displacements);
    unnormalizePoints(coords, val[7], val[3], val[4]);

    // Construct Descriptor objects
    std::vector<float>::iterator hogIteratorBegin = val.begin() + TRACK_INFO_LEN + 2 * TRACK_LEN;
    std::vector<float> hog(hogIteratorBegin, hogIteratorBegin + HOG_DIM);
    std::vector<float>::iterator hofIteratorBegin = hogIteratorBegin + HOG_DIM;
    std::vector<float> hof(hogIteratorBegin, hogIteratorBegin + HOG_DIM);

    std::vector<float>::iterator mbhxIteratorBegin = hofIteratorBegin + HOF_DIM;
    std::vector<float> mbhx(mbhxIteratorBegin, mbhxIteratorBegin + MBHX_DIM);

    std::vector<float>::iterator mbhyIteratorBegin = mbhxIteratorBegin + MBHX_DIM;
    std::vector<float> mbhy(mbhyIteratorBegin, mbhyIteratorBegin + MBHY_DIM);

    if (mbhyIteratorBegin + MBHY_DIM != val.end()) {
      std::cout << "====================================Bug=======================================" << std::endl;
      std::cout << filename << std::endl;
    }
    // Construct Tracks
    tracks.push_back(
      track(static_cast<int>(val[2]), val[3], val[4], val[5], val[6], val[7], val[8], val[9], val[10], val[11], displacements, coords, hog, hof, mbhx, mbhy));
  }
}
