# set the binaries that have to be built
TARGETS := ShowTrajectories ConstructCodebook DumpDominantTrajectoryCluster DrawClusters ClusterTraj DenseTrack Video

protoc_middleman:
	protoc --cpp_out=. dump.proto
	@touch protoc_middleman

DumpDominantTrajectoryCluster: DumpDominantTrajectoryCluster.cpp protoc_middleman
	pkg-config --cflags protobuf
	c++ DumpDominantTrajectoryCluster.cpp dump.pb.cpp -o DumpDominantTrajectoryCluster -std=c++11 `pkg-config --cflags --libs protobuf`

ConstructCodebook: ConstructCodebook.cpp protoc_middleman
	pkg-config --cflags protobuf
	c++ ConstructCodebook.cpp dump.pb.cpp -o ConstructCodebook -std=c++11 -I /home/pighead/Documents/vlfeat -L /home/pighead/Documents/vlfeat/bin/glnxa64/ -lvl `pkg-config --cflags --libs protobuf`	

#Cluster: ClusterTraj.o DenseTrack.h protoc_middleman
#	pkg-config --cflags protobuf
#	c++ ClusterTraj.cpp dump.pb.cpp -o ClusterTraj $(CXXFLAGS) $(LDFLAGS)`pkg-config --cflags --libs protobuf`	

ShowTrajectories: ShowTrajectories.cpp protoc_middleman
	pkg-config --cflags protobuf
	c++ ShowTrajectories.cpp dump.pb.cpp -o ShowTrajectories -std=c++11 `pkg-config --cflags --libs protobuf`

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
	avformat avdevice avutil avcodec swscale

# set some flags and compiler/linker specific commands
CXXFLAGS = -pipe -D __STDC_CONSTANT_MACROS -D STD=std -Wall $(CXXFLAGS_$(BUILD)) -I. -I/opt/include -std=c++11
CXXFLAGS_debug := -ggdb
CXXFLAGS_release := -O3 -DNDEBUG -ggdb
LDFLAGS = -L/opt/lib -pipe -Wall $(LDFLAGS_$(BUILD))
LDFLAGS_debug := -ggdb
LDFLAGS_release := -O3 -ggdb

include make/generic.mk
