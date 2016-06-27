#!/bin/bash
################################ Set up configurations for Dataset ##############################

################################ Dataset-specific configurations ################################

# For each dataset
# 1. define your 'label to integer' mapping in ParserHelpers.h line 20 and 
# 2. add a case to ParseTracks.cpp around line 30
# >> make ParseTracks
_DATASET="UCFSports"	

# Path to the videos, depending on how you organize them
_VIDEO_LOCATION="$_DATASET/original/"	

# Total number of videos in this dataset, over all classes
_NUM_VIDEOS=150

_VIDEO_TYPE=".avi"

# Categories and number of videos under each category in the dataset
declare -A CATEGORIES
# UCFSports
#CATEGORIES=(['Diving-Side']=5 ['Golf-Swing-Back']=5 ['Golf-Swing-Front']=8 ['Golf-Swing-Side']=5)
CATEGORIES=(['Diving-Side']=14 ['Golf-Swing-Back']=5 ['Golf-Swing-Front']=8 ['Golf-Swing-Side']=5 ['Kicking-Front']=10 ['Kicking-Side']=10 ['Lifting']=6 ['Riding-Horse']=12 ['Run-Side']=13 ['SkateBoarding-Front']=12 ['Swing-Bench']=20 ['Swing-SideAngle']=13 ['Walk-Front']=22)

# sJHMDB
#CATEGORIES=(['catch']=1)
#CATEGORIES=(['catch']=5 ['climb_stairs']=1 ['golf']=12 ['jump']=8 ['kick_ball']=8 ['pick']=8 ['pullup']=13 ['push']=10 ['run']=7 ['shoot_ball']=6 ['swing_baseball']=7 ['walk']=4)
#CATEGORIES=(['catch']=25 ['climb_stairs']=22 ['golf']=30 ['jump']=18 ['kick_ball']=16 ['pick']=19 ['pullup']=17 ['push']=18 ['run']=17 ['shoot_ball']=14 ['swing_baseball']=16 ['walk']=15)

# kth
#CATEGORIES=(['boxing']=100 ['handclapping']=99 ['handwaving']=100 ['walking']=100 ['jogging']=100 ['running']=100)
################################ Strategy-related configurations ################################

################################ Regarding getVideos.sh #########################################

# Which strategy to extract Dense Tracks: Improved Dense Tracks (2013) or Dense Tracks (2011)
_EXTRACTION="idt"

################################ Regarding makeArchives.sh ######################################

# Path to dense track extraction binaries, at least one of them needs to be set
_PATH_TO_IDT_BINARY="../improved_trajectory_release/DenseTrackStab"
#_PATH_TO_DT_BINARY="./DenseTrack"

# Place the archives here, tracks are unprocessed and not sorted in any order
_ARCHIVE_LOCATION="$_VIDEO_LOCATION/RawTracks/"

################################ Regarding superTracks.sh ########################################

# Path to parallel spectral clustering binaries
_PATH_TO_PSPECTRAL="../pspectralclustering/"
_DISTANCE_TO_SIMILARITY=$_PATH_TO_PSPECTRAL"distance_to_similarity"
_EVD=$_PATH_TO_PSPECTRAL"evd"
_KMEANS=$_PATH_TO_PSPECTRAL"kmeans"

# Max number of processes each binary can dispatch
_NUM_DISTANCE_TO_SIMILARITY_WORKERS=8
_NUM_EVD_WORKERS=10
_NUM_KMEANS_WORKERS=4

# Max number of trajectories each video can own. If the original number of trajectories exceed
# this amount, we will random sample $_RAW_TRACK_CAP tracks and discard the rest
_RAW_TRACK_CAP=6000

# Gamma value defined in [2] page 4 equation 4, we experiment with different values of gamma
# Values need to be seperated by spaces
_TEMPORAL_MISALIGNMENT_PENALTY=(0.05)

# Maximum number of clusters pspectralclustering can make, the number of non-empty clusters after 
# pspectralclustering tend to be much lower than this amount. This value affects the clustering result.
_MAX_NUM_CLUSTER=(300)

# Location to store the results of spectral clustered trajectories
#
# BuildGraph.exe will place 
#   sorted trajectories in $_CLUSTERED_TRACKS_PATH as {video name}_sortedTrajectories.out in $_CLUSTERED_TRACKS_PATH/r=../
#   adjacency matrix of the sorted tracks in $_CLUSTERED_TRACKS_PATH/r=../
#
# MergeTracks.exe will place: 
#   supertracks (archive form) in $_CLUSTERED_TRACKS_PATH/r=../c=../archives/
#   supertracks (txt form) in $_CLUSTERED_TRACKS_PATH/r=../c=../supertracks/
#   edges between supertracks (txt form) in $_CLUSTERED_TRACKS_PATH/r=../c=../edges/
#
# LargestClusterExtraction.exe will place:
#   tracks in the largest cluster (archive form) in $_CLUSTERED_TRACKS_PATH/r=../c=../largestCluster/
_CLUSTERED_TRACKS_PATH="$_VIDEO_LOCATION/ClusteredTrajectories/sample=$_RAW_TRACK_CAP/"

################################ Regarding looScore.sh ########################################

# Location of trajectories/super tracks we're gonna use for codebook construction, 
# NOTE: it can be raw tracks, tracks from the largest clusters, super tracks, or any other form of tracks regardless.
#       But they need to be in boost::archive form.
# For [1], point this to $_ARCHIVE_LOCATION, because we just want raw trajectories
# For [2], point this to $_CLUSTERED_TRACKS_PATH, because we want to use super tracks
# !important : Also change _TRAINING_TRAJECTORY_LOCATION in looScore.sh to the correct archive location (line 22)
_TRAINING_TRAJECTORY_LOCATION=$_CLUSTERED_TRACKS_PATH


# Channels to be used in Bag of Words procedure. 
# If you set $_CHANNEL to be other values like MBH, HOG ..., you would need to modify 
# 1. BagOfWords.cpp lines 290 and 245
# 2. ChiSquaredSVM.cpp lines 6, 56, 167 and 249.
_CHANNEL=All

# Sample $_CODEBOOK_SAMPLE trajectories from to build codebooks
# In the Dense Track [1] literature, this is set to 100,000
_CODEBOOK_SAMPLE=4000

# Dimension of each codebook
# In the Dense Track literature [1], this is set to 4,000
_CODEBOOK_CENTERS=500

# Location to put training set features, svm will read from here
_TRAININGSET_DESTINATION="LargestCluster/"

_PATH_TO_LIBSVM="../libsvm/"
_SVM_TRAIN=$_PATH_TO_LIBSVM"svm-train"

################################ Unimportant configurations, just for console display formatting ########################################
progress=0
w="=========="
bar="[=======================================================================]"

# Open issues
#
# There are some memory allocation bugs in OpenCV, 
# sometimes the DenseTrackStab binary segfaults when trying to read frames from videos 
# (during extracting Dense Trajectories, visualizing clusters and trajectories...), 
# especially when the file type is .avi (likely because of its messed up encoding scheme).
# When this happens and you're certain that the video isn't corrupted, 
# try converting the video file into another file type such as flv and then retry.
# >> avconv -i video.avi video.flv 
#
# When running loo.sh, if BagOfWords starts to seg fault
# Change reservoir sampling to random sampling in BagOfWords.cpp 
# (around line 280, explanation included) and retry.