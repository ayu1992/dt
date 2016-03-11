#!/bin/bash
# Computes dense trajectories of a video
# Cluster the trajectories
# Runs spectral clustering on the trajectories
# For a specified NUM_CLUSTERS, generate track dumps of all videos

# protoc -I=proto
       
declare -A CATEGORIES
#CATEGORIES=( ['BackGolf']=5 ['Diving']=14 ['FrontGolf']=8 ['FrontKick']=10 ['Horse']=12 ['Lifting']=6 ['Running']=13 ['SideGolf']=5 ['SideKick']=10 ['SideSwing']=13 ['Skateboard']=12 ['SwingBench']=20 ['Walking']=22)
#CATEGORIES=(['Diving']=8 ['Horse']=8 ['Lifting']=6 ['SwingBench']=8 ['SideSwing']=9 ['Running']=7 ['Skateboard']=8 ['FrontGolf']=8 ['Walking']=8)
# problems iwth backgolf data!
CATEGORIES=(['Lifting']=3)
NUM_CLUSTERS=2

PARAM_R=0.03

# Output path
OUT_PATH="ClusteredTrajectories/r="
OUT_PATH+=$PARAM_R
OUT_PATH+="/c="
OUT_PATH+=$NUM_CLUSTERS
OUT_PATH+="/DrawBB/"

# Wipe out previously generated dump

# Side swing 8 has 0 tracks
# Running 13 has 0 tracks
# Horses 9 got stuck in some loops
# Dump out dominant trajectories for these videos, dest:TrajectoryDump.data
make ClusterTraj
make DumpDominantTrajectoryCluster
make DrawClusters
make LocalizationScoreForVideo

START_TIME=$(date +%s)

rm $OUT_PATH"scores.txt"

for CATEGORY in "${!CATEGORIES[@]}"			# '!' expands keys, no '!' expands values
do
	for (( vid=3; vid <= ${CATEGORIES[$CATEGORY]}; vid++))
	do 
		
		if [ $vid -eq 5 ] && [ $CATEGORY == 'FrontGolf' ] 
			then
				continue
		fi

		if [ $vid -eq 8 ] && [ $CATEGORY == 'FrontGolf' ] 
			then
				continue
		fi

		if [ $vid -eq 2 ] && [ $CATEGORY == 'SideGolf' ] 
			then
				continue
		fi

		if [ $vid -eq 3 ] && [ $CATEGORY == 'SideSwing' ] 
			then
				continue
		fi

		if [ $vid -eq 5 ] && [ $CATEGORY == 'SideSwing' ] 
			then
				continue
		fi

		if [ $vid -eq 8 ] && [ $CATEGORY == 'SideSwing' ] 
			then
				continue
		fi

		INPUT_VIDEO=$vid
		VIDEO_NAME="InputVideos/" 
		VIDEO_NAME+=$CATEGORY
		VIDEO_NAME+="/" 
		VIDEO_NAME+=$INPUT_VIDEO 
		VIDEO_NAME+=".features"
		echo $VIDEO_NAME
		
		rm $OUT_PATH"sortedTrajectories.out"
		rm $OUT_PATH"result.txt"
	
		./ClusterTraj $VIDEO_NAME $OUT_PATH $PARAM_R

		mpiexec -n 2 ../pspectralclustering/distance_to_similarity --input dij.txt --output similarity.txt
		NUM_SPACE=$(($NUM_CLUSTERS * 3))
		mpiexec -n 10 ../pspectralclustering/evd --eigenvalue $NUM_CLUSTERS --eigenspace $NUM_SPACE --input similarity.txt --eigenvalues_output eigenvalues.txt --eigenvectors_output eigenvectors.txt
		mpiexec -n 4 ../pspectralclustering/kmeans --num_clusters $NUM_CLUSTERS --input eigenvectors.txt --output $OUT_PATH"result.txt"
		
		# Find video with few tracks: Lift 2
		# Modify dense track to output coords
		# Compare coords with de-normalized ones
		# generate dense track for videos with original resolution
		# localization score
		#./LocalizationScoreForVideo $OUT_PATH "InputVideos/"$CATEGORY"/" $NUM_CLUSTERS $vid 0.1

		#./DumpDominantTrajectoryCluster $OUT_PATH $CATEGORY $vid $NUM_CLUSTERS
		
		IN_PATH="InputVideos/"
		IN_PATH+=$CATEGORY
		IN_PATH+="/"

		./DrawClusters $OUT_PATH $IN_PATH $vid $NUM_CLUSTERS 

#		rm similarity.txt
#		rm dij.txt
#		rm eigenvectors.txt
#		rm eigenvalues.txt
	done
done

END_TIME=$(date +%s)
EXECUTION_TIME=$(($END_TIME - $START_TIME))
echo "Execution time: $EXECUTION_TIME seconds"




