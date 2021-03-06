# set the binaries that need to be built
TARGETS := DenseTrack Video
#DrawClusters DrawTracks 

ParseTracks: ParseTracks.cpp
	g++ ParseTracks.cpp -o ParseTracks -I /home/hydralisk/Documents/boost_1_60_0 /usr/local/lib/libboost_serialization.a -std=c++11

symmetricCheck: symmetricCheck.cpp
	g++ symmetricCheck.cpp -o symmetricCheck -I /home/hydralisk/Documents/boost_1_60_0 /usr/local/lib/libboost_serialization.a -std=c++11

LargestClusterExtraction: LargestClusterExtraction.cpp
	g++ LargestClusterExtraction.cpp -o LargestClusterExtraction -I /home/hydralisk/Documents/boost_1_60_0 /usr/local/lib/libboost_serialization.a -std=c++11

BuildGraph: BuildGraph.cpp
	g++ BuildGraph.cpp -o BuildGraph -I /home/hydralisk/Documents/boost_1_60_0 /usr/local/lib/libboost_serialization.a -std=c++11

GetCoordsForClusters: GetCoordsForClusters.cpp
	g++ GetCoordsForClusters.cpp -o GetCoordsForClusters -I /home/hydralisk/Documents/boost_1_60_0 /usr/local/lib/libboost_serialization.a -std=c++11

GenerateSubsets: GenerateSubsets.cpp
	g++ GenerateSubsets.cpp -o GenerateSubsets -I /home/hydralisk/Documents/boost_1_60_0 /usr/local/lib/libboost_serialization.a -std=c++11	

MergeTracks: MergeTracks.cpp
	g++ MergeTracks.cpp -o MergeTracks -I /home/hydralisk/Documents/boost_1_60_0 /usr/local/lib/libboost_serialization.a -I /home/hydralisk/Documents/vlfeat -L /home/hydralisk/Documents/vlfeat/bin/glnxa64/ -lvl -std=c++11

BagOfWords: BagOfWords.cpp
	g++ BagOfWords.cpp -o BagOfWords -I /home/hydralisk/Documents/boost_1_60_0 /usr/local/lib/libboost_serialization.a -I /home/hydralisk/Documents/vlfeat -L /home/hydralisk/Documents/vlfeat/bin/glnxa64/ -lvl -std=c++11

ChiSquaredSVM: ChiSquaredSVM.cpp
	g++ ChiSquaredSVM.cpp -o ChiSquaredSVM -I /home/hydralisk/Documents/boost_1_60_0 /usr/local/lib/libboost_serialization.a -I /home/hydralisk/Documents/vlfeat -L /home/hydralisk/Documents/vlfeat/bin/glnxa64/ -lvl -std=c++11

countActualClusters: countActualClusters.cpp
	g++ countActualClusters.cpp -o countActualClusters -std=c++11

# set the build configuration set 
BUILD := release

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
