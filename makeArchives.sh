DATASET="UCFSports"
VIDEODATAPATH="$DATASET/original"
declare -A CATEGORIES
CATEGORIES=(['Diving-Side']=14 ['Golf-Swing-Back']=5 ['Golf-Swing-Side']=5 ['Kicking-Front']=10 ['Kicking-Side']=10 ['Lifting']=6 ['Riding-Horse']=12 ['Run-Side']=13 ['SkateBoarding-Front']=12 ['Swing-Bench']=20 ['Swing-SideAngle']=13 ['Walk-Front']=22)
#CATEGORIES=(['Riding-Horse']=10)
#CATEGORIES=(['Swing-SideAngle']=4)

#make DrawClusters
#make DominantClusterFilter
rm ParseTracks
make ParseTracks

START_TIME=$(date +%s)

ARCHIVE_LOCATION="NoClustering/"

for CATEGORY in "${!CATEGORIES[@]}"	
do 
	for ((vid=1; vid<=${CATEGORIES[$CATEGORY]}; vid++))
	do
		VIDEO_LOCATION="$VIDEODATAPATH/$CATEGORY/$vid.features"
		# DenseTrack -> Archives
		./ParseTracks $DATASET $VIDEO_LOCATION $ARCHIVE_LOCATION $CATEGORY $vid
	done
done

# Join Archives

END_TIME=$(date +%s)
EXECUTION_TIME=$(($END_TIME - $START_TIME))
echo "Execution time: $EXECUTION_TIME seconds"
