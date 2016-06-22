source ./configurations.sh
progress=0

make ParseTracks

# We expect Dense track features from the videos to be already placed in 
# Path to dataset/category/

# This script will read from Path to dataset/category/*.features and place its
# outputs in Path to archive/*.out
mkdir -p $_ARCHIVE_LOCATION

for CATEGORY in "${!CATEGORIES[@]}"	
do 
	for ((vid=1; vid<=${CATEGORIES[$CATEGORY]}; vid++))
	do
		((progress++))
		./ParseTracks $_DATASET "$_VIDEO_LOCATION$CATEGORY/$vid.features" $_ARCHIVE_LOCATION $CATEGORY $vid
			
		pd=$(($progress * 73 / $_NUM_VIDEOS))
		printf "\r%3d.%1d%% %.${pd}s" $(( $progress * 100 / $_NUM_VIDEOS )) $(( ($progress * 1000 / $_NUM_VIDEOS) % 10 )) $bar
	done
done
