/**
 * Helper functions around boost library.
 */
#include "ParserHelpers.h"
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/algorithm/string.hpp>

// This struct mirrors the OpenCV::Point2d class and
// adds support to boost::serialization.
struct point {
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & x;
    ar & y;
  }
  float x;
  float y;
  
  point() {x = 0.0; y = 0.0;};
  point(const float x, const float y): x(x), y(y) {};

  // multiplication that mutates self
  point& operator*=(float s) {
    x *= s;
    y *= s;
    return *this;
  }
};

// Operator overload
point operator+(const point& p1, const point& p2)
{
  return point(p1.x + p2.x, p1.y + p2.y);
}

point operator-(const point& p1, const point& p2)
{
  return point(p1.x - p2.x, p1.y - p2.y);
}

point operator*(const point& p1, const float s)
{
  return point(p1.x * s, p1.y * s);
}

point operator/(const point& p1, const float s)
{
  return point(p1.x / s, p1.y / s);
}

// Unnormalizes displacements into original coordinates. 
// Saves result in original vector 'normalizedDisplacements'. 
// First argument 'normalizedDisplacements' contains displacements of coordinates
//   (pair-wise subtraction and normalization of coordinates, see Descriptors.h isValid())
void unnormalizePoints(
  std::vector<point>& normalizedDisplacements,
  const float trajectoryLength, 
  const float mean_x, 
  const float mean_y) {

  for (point& p : normalizedDisplacements) p = p * trajectoryLength;
  // Infer the last point.
  normalizedDisplacements.emplace_back(mean_x, mean_y);
  for (size_t i = 0; i < normalizedDisplacements.size() - 1; ++i) {
    normalizedDisplacements.back() = 
        normalizedDisplacements.back() + 
            (normalizedDisplacements[i] * (static_cast<float>(i + 1) / normalizedDisplacements.size()));
  }
  for (int i = normalizedDisplacements.size() - 2; i >= 0; --i) normalizedDisplacements[i] = normalizedDisplacements[i + 1] - normalizedDisplacements[i];
}

// An serializable object that represents a dense trajectory.
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
  float var_x;   // variance of the coords
  float var_y;  
  float length;  // sum of Eucledian distance of Points on the trajectory
  float scale;   // The trajectory is computed on which scale
  float x_pos;
  float y_pos;
  float t_pos;
  std::vector<point> displacements;   // dimension: 15 x 2
  std::vector<point> coords;          // dimension: 16 x 2
  std::vector<float> hog;             // features
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

  track(const track& t) : endingFrame(t.endingFrame), mean_x(t.mean_x), mean_y(t.mean_y), var_x(t.var_x), var_y(t.var_y), 
  length(t.length), scale(t.scale), x_pos(t.x_pos), y_pos(t.y_pos), t_pos(t.t_pos), displacements(t.displacements), coords(t.coords), 
  hog(t.hog), hof(t.hof), mbhx(t.mbhx), mbhy(t.mbhy) {}
};

// Extracts the coordinates from a trajectory
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

// Extracts the displacements from a trajectory
struct DisplacementsGetter {
  std::vector<float> operator()(const track& t) {
    std::vector<float> ret(2 * t.displacements.size());
    for (size_t i = 0; i < t.displacements.size(); ++i) {
      ret[2 * i] = t.displacements[i].x;
      ret[2 * i + 1] = t.displacements[i].y;
    }
    return ret;
  }
  static constexpr int dimension = 30;
};

// Extracts hog features from a trajectory
struct HogGetter {
  const std::vector<float>& operator()(const track& t) { return t.hog; }
  static constexpr int dimension = 96;
};

// Extracts hof features from a trajectory
struct HofGetter {
  const std::vector<float>& operator()(const track& t) { return t.hof; }
  static constexpr int dimension = 108;
};

// Extracts mbhx features from a trajectory
struct MbhXGetter {
  const std::vector<float>& operator()(const track& t) { return t.mbhx; }
  static constexpr int dimension = 96;
};

// Extracts mbhy features from a trajectory
struct MbhYGetter {
  const std::vector<float>& operator()(const track& t) { return t.mbhy; }
  static constexpr int dimension = 96;
};

// A serializable object that holds a list of trajectories
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

// A serializable object that represents a video.
// Contains trajectories, class label and video information
class videoRep {
private:
	friend class boost::serialization::access;
	friend std::ostream & operator<<(std::ostream &os, const videoRep& video);
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & _tracks;
		ar & _classLabel;			
    ar & _vid;            
    ar & _videoWidth;     
    ar & _videoHeight;
	}
	// a video is represented by One trajectory cluster/list
	trackList _tracks;
  // label of action class
	int _classLabel;	    
  // id of the video, in this project, videos are named by {class name}/vid.{video type}
	int _vid;           
  // frame dimensions   
  int _videoWidth;    
  int _videoHeight;
  /* TODO: parse and store bounding boxes in here*/

public:
	videoRep() {};
	videoRep(
    const trackList& tracks,
    const int classLabel, 
    const int vid, 
    const int videoWidth, 
    const int videoHeight): 
  _tracks(tracks), 
  _classLabel(classLabel), 
  _vid(vid),
  _videoWidth(videoWidth), 
  _videoHeight(videoHeight) {}

	const trackList& getTrackList() const {return _tracks;}
	const int classLabel() const {return _classLabel;}
	const int vid() const {return _vid;}
  const int videoWidth() const {return _videoWidth;}
  const int videoHeight() const {return _videoHeight;}
};

// Restores object from an archive file
void restoreTrackList(const std::string& path, trackList& tList) {
    std::ifstream ifs(path);    // contains A video
    boost::archive::binary_iarchive ia(ifs);
    ia >> tList;
}

// Restores object from an archive file
void restoreVideoRep(const std::string& path, videoRep& video) {
    std::ifstream ifs(path);    // contains A video
    boost::archive::binary_iarchive ia(ifs);
    ia >> video;
}

// Object to store bounding boxes
struct Box {
  point UpperLeft;
  float width;
  float height;
};

// Check if a point is inside a bounding box
bool isInBox(const point& p, const Box& box) {
  float diffx = p.x - box.UpperLeft.x;
  float diffy = p.y - box.UpperLeft.y;
  return (diffx >= 0 && diffx <= box.width) && (diffy >= 0 && diffy <= box.height);
}

// Parses bounding boxes from text file
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
    boxes.emplace_back(Box{point(val[0], val[1]), val[2], val[3]});
  }
  return boxes;
}

// Reads and parses dense trajectories from file.
// Stores trajectories in 3rd argument 'tracks'
void parseFeaturesToTracks(
  const std::string& filename, 
  std::vector<std::string>& trajInStrings, 
  std::vector<track>& tracks, 
  int& videoWidth, 
  int& videoHeight) {
  
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
    std::vector<float> hof(hofIteratorBegin, hofIteratorBegin + HOF_DIM);

    std::vector<float>::iterator mbhxIteratorBegin = hofIteratorBegin + HOF_DIM;
    std::vector<float> mbhx(mbhxIteratorBegin, mbhxIteratorBegin + MBHX_DIM);

    std::vector<float>::iterator mbhyIteratorBegin = mbhxIteratorBegin + MBHX_DIM;
    std::vector<float> mbhy(mbhyIteratorBegin, mbhyIteratorBegin + MBHY_DIM);

    // Construct Tracks
    tracks.push_back(
      track(static_cast<int>(val[2]), val[3], val[4], val[5], val[6], val[7], val[8], val[9], val[10], val[11], displacements, coords, hog, hof, mbhx, mbhy));
  }
}

// Parses a string like "3:5.5" into a pair<int, float>.
std::pair<int, float> parseIntoPair(const std::string& s) {
  std::vector<std::string> tmp; // tmp would have 2 elements
  boost::split(tmp, s, boost::is_any_of(":"));

  if (tmp.size() != 2) {
    std::cout << "Failed to parse pair: " << s << std::endl;
    return {-1, -1};
  }
  return {std::stoi(tmp[0]), std::stof(tmp[1])};
}

// Given a list of trajectoires 'tList', writes all of their coordinates into file
void writeCoordsToFile(const std::string& path, const trackList& tList) {
  std::ofstream ots(path);
  {
    for (const auto& trackIdTrackPair : tList.tracks()) {
      const track& t = trackIdTrackPair.second;
      ots << t.endingFrame << " ";
      for (size_t i = 0; i < t.coords.size(); ++i) {
        if ( i != 0) ots << " ";
        ots << t.coords[i].x << " " << t.coords[i].y;
      }
      ots << std::endl;
    }
  }
  ots.close();
}

// Randomly sample numSamples from source
template<typename T>
std::vector<T> randomSample(const std::vector<T>& source, const int numSamples) {
  std::vector<T> copy(source);
  std::random_shuffle(copy.begin(), copy.end());
  std::vector<T> samples(copy.begin(), copy.begin() + numSamples);
  return samples;
}

// Checks if a particular kind of feature is missing from a trajectory
template <typename Functor>
bool checkIsFeatureEmpty(const track& t) {
  return !Functor()(t).size();
}

bool checkContainsEmptyFeature(const track& t) {
  return 
    checkIsFeatureEmpty<DisplacementsGetter>(t) || 
      checkIsFeatureEmpty<HogGetter>(t) ||
      checkIsFeatureEmpty<HofGetter>(t) ||
      checkIsFeatureEmpty<MbhXGetter>(t)||
      checkIsFeatureEmpty<MbhYGetter>(t);
}

// Checks every channels of a trajectory for NaN values
void checkContainNaN(const std::vector<track>& tracks) {
  std::cout << "Checking for dirty data" << std::endl;
  bool isDirty;
  for (const auto& t : tracks) {
    isDirty = false;
    std::for_each(t.displacements.begin(), t.displacements.end(), [&isDirty](const point& p){
      if (!std::isfinite(p.x) || !std::isfinite(p.y)) isDirty = true;
    });

    if(std::any_of(t.hog.begin(), t.hog.end(), [](const float& f){return !std::isfinite(f);})
    || std::any_of(t.hof.begin(), t.hof.end(), [](const float& f){return !std::isfinite(f);})
    || std::any_of(t.mbhx.begin(), t.mbhx.end(), [](const float& f){return !std::isfinite(f);})
    || std::any_of(t.mbhy.begin(), t.mbhy.end(), [](const float& f){return !std::isfinite(f);})) {
      isDirty = true;
    }
    if (isDirty) std::cout << "===========================Dirty data============================" << std::endl;
  }
}