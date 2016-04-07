# set the binaries that have to be built
#TARGETS := ShowTrajectories ConstructCodebook DumpDominantTrajectoryCluster LocalizationScoreForVideo DominantClusterFilter DrawClusters ClusterTracks DenseTrack Video
TARGETS := DenseTrack Video
################################## To be deprecated
#DumpDominantTrajectoryCluster: DumpDominantTrajectoryCluster.cpp protoc_middleman
#	pkg-config --cflags protobuf
#	c++ DumpDominantTrajectoryCluster.cpp dump.pb.cpp -o DumpDominantTrajectoryCluster -std=c++11 `pkg-config --cflags --libs protobuf`

#ConstructCodebook: ConstructCodebook.cpp protoc_middleman
#	pkg-config --cflags protobuf
#	c++ ConstructCodebook.cpp dump.pb.cpp -o ConstructCodebook -std=c++11 -I /home/pighead/Documents/vlfeat -L /home/pighead/Documents/vlfeat/bin/glnxa64/ -lvl `pkg-config --cflags --libs protobuf`	

#DenseTrackToProto: DenseTrackToProto.cpp protoc_middleman
#	pkg-config --cflags protobuf
#	c++ DenseTrackToProto.cpp dump.pb.cpp -o DenseTrackToProto -std=c++11 `pkg-config --cflags --libs protobuf`		

#ShowTrajectories: ShowTrajectories.cpp protoc_middleman
#	pkg-config --cflags protobuf
#	c++ ShowTrajectories.cpp dump.pb.cpp -o ShowTrajectories -std=c++11 `pkg-config --cflags --libs protobuf`
############################################
# Multiple Archives -> Codeook and features

# Archive -> BoW
BagOfWords: BagOfWords.cpp
	g++ BagOfWords.cpp -o BagOfWords -I /home/hydralisk/Documents/boost_1_60_0 /usr/local/lib/libboost_serialization.a -I /home/hydralisk/Documents/vlfeat -L /home/hydralisk/Documents/vlfeat/bin/glnxa64/ -lvl -std=c++11

locateNaN: locateNaN.cpp
	g++ locateNaN.cpp -o locateNaN -I /home/hydralisk/Documents/boost_1_60_0 /usr/local/lib/libboost_serialization.a -std=c++11

# DenseTrack -> Archive
ParseTracks: ParseTracks.cpp
	g++ ParseTracks.cpp -o ParseTracks -I /home/hydralisk/Documents/boost_1_60_0 /usr/local/lib/libboost_serialization.a -std=c++11

# DenseTrack -> Cluster tracks -> Archive
ClusterTracks: ClusterTracks.cpp
	g++ ClusterTracks.cpp -o ClusterTracks -I /home/hydralisk/Documents/boost_1_60_0 /usr/local/lib/libboost_serialization.a -std=c++11

DominantClusterFilter: DominantClusterFilter.cpp
	g++ DominantClusterFilter.cpp -o DominantClusterFilter -I /home/hydralisk/Documents/boost_1_60_0 /usr/local/lib/libboost_serialization.a -std=c++11		

LocalizationScoreForVideo: LocalizationScoreForVideo.cpp
	g++ LocalizationScoreForVideo.cpp -o LocalizationScoreForVideo -I /home/hydralisk/Documents/boost_1_60_0 /usr/local/lib/libboost_serialization.a -std=c++11	


# set the build configuration set 
BUILD := release
#BUILD := debug

# set bin and build dirs
BUILDDIR := .build_$(BUILD)
BINDIR := $(BUILD)

# libraries 
LDLIBS = $(addprefix -l, $(LIBS) $(LIBS_$(notdir $*)))
LIBS := \
	opencv_core opencv_highgui opencv_video opencv_imgproc \
	avformat avdevice avutil avcodec swscale boost_serialization

# set some flags and compiler/linker specific commands
CXXFLAGS = -pipe -D __STDC_CONSTANT_MACROS -D STD=std -Wall $(CXXFLAGS_$(BUILD)) -I. -I/opt/include -std=c++11
CXXFLAGS_debug := -ggdb
CXXFLAGS_release := -O3 -DNDEBUG -ggdb
LDFLAGS = -L/opt/lib -pipe -Wall $(LDFLAGS_$(BUILD))
LDFLAGS_debug := -ggdb
LDFLAGS_release := -O3 -ggdb

include make/generic.mk
