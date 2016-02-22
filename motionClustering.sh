#!/bin/bash
# Computes dense trajectories of a video
# Cluster the trajectories
# Runs spectral clustering on the trajectories

#VIDEO_NAME="person01_boxing_d1_uncomp.avi"
#protoc -I=proto

CATEGORIES=('Diving' 'Lifting' 'Running')
TYPE="TestingSet"
vid=3
NUM_CLUSTERS=1
PARAM_R=3
# NOTE: Running/3.avi was 4.avi

# Output path
OUT_PATH="ClusteredTrajectories/r="
OUT_PATH+=$PARAM_R
OUT_PATH+="/c="
OUT_PATH+=$NUM_CLUSTERS
OUT_PATH+="/"

# Wipe out previously generated dump
rm $OUT_PATH$TYPE"TrajectoryDump.data"


# Dump out dominant trajectories for these videos, dest:TrajectoryDump.data
make ClusterTraj
make DumpDominantTrajectoryCluster

for CATEGORY in "${CATEGORIES[@]}"
do
	START_TIME=$(date +%s)
	INPUT_VIDEO=$vid
	VIDEO_NAME="InputVideos/" 
	VIDEO_NAME+=$CATEGORY
	VIDEO_NAME+="/" 
	VIDEO_NAME+=$INPUT_VIDEO 
	VIDEO_NAME+=".avi"
	echo $VIDEO_NAME

	../improved_trajectory_release/release/DenseTrackStab $VIDEO_NAME > out.features
	./ClusterTraj
	mpiexec -n 2 ../pspectralclustering/distance_to_similarity --input dij.txt --output similarity.txt
	NUM_SPACE=5	
	mpiexec -n 10 ../pspectralclustering/evd --eigenvalue $NUM_CLUSTERS --eigenspace $NUM_SPACE --input similarity.txt --eigenvalues_output eigenvalues.txt --eigenvectors_output eigenvectors.txt
	mpiexec -n 4 ../pspectralclustering/kmeans --num_clusters $NUM_CLUSTERS --input eigenvectors.txt --output result.txt
	./DumpDominantTrajectoryCluster $OUT_PATH $CATEGORY $vid $NUM_CLUSTERS $TYPE
	
	END_TIME=$(date +%s)
	EXECUTION_TIME=$(($END_TIME - $START_TIME))
	echo "Execution time: $EXECUTION_TIME seconds"
done










#	
#	INPUT_VIDEO="swing"
#	PSPEC_PATH="../pspectralclustering"
# typically set to 2*Num_clusters

#		OUTPUT_NAME="OutVideos/"
#		OUTPUT_NAME+=$INPUT_VIDEO
#		OUTPUT_NAME+="_"
#		OUTPUT_NAME+=$NUM_CLUSTERS
#		OUTPUT_NAME+=".avi"
#		./DrawClusters $VIDEO_NAME $NUM_CLUSTERS $OUTPUT_NAME	

#
#
#




