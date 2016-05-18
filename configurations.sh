#### Set up configurations for Dataset ####//  export these variables  

#### Dataset-specific configurations ####

# Fill out a corresponding 'label to integer' mapping in ParserHelpers.h line 20 and add a case 
# to ParseTracks.cpp line 30
_DATASET="UCFSports"	

# Path to the videos, depending on how you organize them
_VIDEO_LOCATION="$DATASET/original"	

# Total number of videos in this dataset, over all classes
_NUM_VIDEOS=150

_VIDEO_TYPE=".vob"

# Categories and number of videos under each category in the dataset
declare -A CATEGORIES
#CATEGORIES=(['Diving-Side']=14 ['Golf-Swing-Back']=5 ['Golf-Swing-Front']=8 ['Golf-Swing-Side']=5 ['Kicking-Front']=10 ['Kicking-Side']=10 ['Lifting']=6 ['Riding-Horse']=12 ['Run-Side']=13 ['SkateBoarding-Front']=12 ['Swing-Bench']=20 ['Swing-SideAngle']=13 ['Walk-Front']=22)
CATEGORIES=(['Kicking-Front']=1)
# Which strategy to extract Dense Tracks: Improved Dense Tracks (2013) or Dense Tracks (2011)
_EXTRACTION="idt"

# declare a vector of Rs'

#### Non-important configurations, just for console display formatting
progress=0
w="=========="
bar="[=======================================================================]"

#### Strategy-related configurations ####

#### Regarding makeArchives.sh ####

# Path to dense track extraction binaries, at least one of them needs to be set
_PATH_TO_IDT_BINARY="../improved_trajectory_release/release/DenseTrackStab"
#_PATH_TO_DT_BINARY="./DenseTrack"

# Place the archives here
_ARCHIVE_LOCATION="RawTracks/"

#### Regarding superTracks.sh ####
_PATH_TO_PSPECTRAL="../pspectralclustering/"
_DISTANCE_TO_SIMILARITY=$_PATH_TO_PSPECTRAL"distance_to_similarity"
_EVD=$_PATH_TO_PSPECTRAL"evd"
_KMEANS=$_PATH_TO_PSPECTRAL"kmeans"

# Max number of trajectories each video can own. If the original number of trajectories exceed
# this amount, we will random sample $_RAW_TRACK_CAP tracks and discard the rest
_RAW_TRACK_CAP=8000

# Values need to be seperated by spaces
_TEMPORAL_MISALIGNMENT_PENALTY_R=(0.4 0.6 0.55)

# Maximum number of clusters pspectralcluster can make
_MAX_NUM_CLUSTER=(500)

# Location to store the results of spectral clustered trajectories
_CLUSTERED_TRACKS_PATH="ClusteredTrajectories/sample="$_RAW_TRACK_CAP"/"

_NUM_DISTANCE_TO_SIMILARITY_WORKERS=8

_NUM_EVD_WORKERS=10

_NUM_KMEANS_WORKERS=4

#### Regarding looScore.sh ####
# Channels to be used in Bag of Words procedure. 
# If you set $_CHANNEL to be other values like MBH, HOG ..., you would need to modify 
# 1. BagOfWords.cpp lines 285 and 237
# 2. ChiSquaredSVM.cpp lines 6, 56, 167 and 249.
_CHANNEL=All

# Sample $_CODEBOOK_SAMPLE trajectories from Training Set to build codebooks
# In the Dense Track literature, this is set to 100,000
_CODEBOOK_SAMPLE=5000

# Dimension of each codebook
# In the Dense Track literature, this is set to 4,000
_CODEBOOK_CENTERS=500