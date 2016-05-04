DATASET="UCFSports"
VIDEODATAPATH="$DATASET/original"
declare -A CATEGORIES
CATEGORIES=(['Diving-Side']=14 ['Golf-Swing-Back']=5 ['Golf-Swing-Front']=8 ['Golf-Swing-Side']=5 ['Kicking-Front']=10 ['Kicking-Side']=10 ['Lifting']=6 ['Riding-Horse']=12 ['Run-Side']=13 ['SkateBoarding-Front']=12 ['Swing-Bench']=20 ['Swing-SideAngle']=13 ['Walk-Front']=22)
numVideos=150
progress=0
bar="[=======================================================================]"

rm ParseTracks
make ParseTracks

START_TIME=$(date +%s)
ARCHIVE_LOCATION="NoClustering/earlyDS/"

for CATEGORY in "${!CATEGORIES[@]}"	
do 
	for ((vid=1; vid<=${CATEGORIES[$CATEGORY]}; vid++))	#for vid in $(seq -f "%03g" 1 ${CATEGORIES[CATEGORY]})
	do
		((progress++))
		VIDEO_LOCATION="$VIDEODATAPATH/$CATEGORY/$vid.features"
		./ParseTracks $DATASET $VIDEO_LOCATION $ARCHIVE_LOCATION $CATEGORY $vid
		pd=$(($progress * 73 / $numVideos))
		printf "\r%3d.%1d%% %.${pd}s" $(( $progress * 100 / $numVideos )) $(( ($progress * 1000 / $numVideos) % 10 )) $bar
	done
done

# run BagOfWords
rm BagOfWords
make BagOfWords

echo "Running Bag of Words"

./BagOfWords $ARCHIVE_LOCATION

END_TIME=$(date +%s)
EXECUTION_TIME=$(($END_TIME - $START_TIME))
echo "Execution time: $EXECUTION_TIME seconds"
