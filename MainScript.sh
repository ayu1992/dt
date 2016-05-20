#!/bin/bash
# TODO: function of this script

#make clean
make all 
#rm BuildGraph
#rm MergeTracks
#rm DrawTracks
#rm GetCoordsForClusters
#rm DrawClusters
#rm countActualClusters
make ParseTracks
make BuildGraph
make MergeTracks
make DrawTracks
make GetCoordsForClusters
make DrawClusters
make countActualClusters
make BagOfWords
make ChiSquaredSVM

# Extract Dense Tracks from videos, output .features files in the same location (next to their video files)
echo $w"Extracting Dense Track"$w
bash getVideos.sh

# Pack them into boost::archives, place in a new folder	
# Turning all of our data (dense tracks) into archive forms makes them 
# easier to access (time efficient) and process (object concept) for future purposes
echo $w"Packing .features into archives"$w
bash makeArchives.sh

echo $w"Build Graph, run spectral and merge trajectories into super tracks"$w
bash superTracks.sh

echo $w"Leave one out cross validation scores"$w
#bash looScore.sh