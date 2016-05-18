source ./configurations.sh
progress=0

mkdir -p $_ARCHIVE_LOCATION

for CATEGORY in "${!CATEGORIES[@]}"	
do 
	for ((vid=1; vid<=${CATEGORIES[$CATEGORY]}; vid++))
	do
		((progress++))
		./ParseTracks $_DATASET $_VIDEO_LOCATION$CATEGORY"/"$vid".features" $_ARCHIVE_LOCATION $CATEGORY $vid
		
		pd=$(($progress * 73 / $_NUM_VIDEOS))
		printf "\r%3d.%1d%% %.${pd}s" $(( $progress * 100 / $numVideos )) $(( ($progress * 1000 / $numVideos) % 10 ))
	done
done
