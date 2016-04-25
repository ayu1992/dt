DATASET="UCFSports/"
declare -A CATEGORIES
#CATEGORIES=(['Diving-Side']=14 ['Golf-Swing-Back']=5 ['Golf-Swing-Side']=5 ['Kicking-Front']=10 ['Kicking-Side']=10 ['Lifting']=6 ['Riding-Horse']=12 ['Run-Side']=13 ['SkateBoarding-Front']=12 ['Swing-Bench']=20 ['Swing-SideAngle']=13 ['Walk-Front']=22)
CATEGORIES=(['Diving-Side']=1)
RESOLUTION=$DATASET"original/"

make BuildGraph
#make LocalizationScoreForVideo
#make DominantClusterFilter
make MergeTracks
make DrawTracks

START_TIME=$(date +%s)

for PARAM_R in 2
do 
	for NUM_CLUSTERS in 600
	do
		# Output path
		OUT_PATH="ClusteredTrajectories/r="
		OUT_PATH+=$PARAM_R
		OUT_PATH+="/c="
		OUT_PATH+=$NUM_CLUSTERS
		OUT_PATH+="/"
		
		for CATEGORY in "${!CATEGORIES[@]}"			# '!' expands keys, no '!' expands values
		do
			#rm $OUT_PATH"scores_"$CATEGORY".txt"
			for (( vid=1; vid <= ${CATEGORIES[$CATEGORY]}; vid++))
			do 

				INPUT_VIDEO=$vid
				VIDEO_NAME=$RESOLUTION
				VIDEO_NAME+=$CATEGORY
				VIDEO_NAME+="/" 
				VIDEO_NAME+=$INPUT_VIDEO 
				VIDEO_NAME+=".features"
				echo $VIDEO_NAME
				
				rm $OUT_PATH"dij.txt"
				rm $OUT_PATH"similarity.txt"
				rm $OUT_PATH"result.txt"
				./BuildGraph $VIDEO_NAME $OUT_PATH $PARAM_R

				mpiexec -n 2 ../pspectralclustering/distance_to_similarity --input $OUT_PATH"dij.txt" --output $OUT_PATH"similarity.txt"
				NUM_SPACE=$(($NUM_CLUSTERS * 3))
				mpiexec -n 1 ../pspectralclustering/evd --arpack_iterations 1000 --arpack_tolerance 0.000001 --eigenvalue $NUM_CLUSTERS --eigenspace $NUM_SPACE --input $OUT_PATH"similarity.txt" --eigenvalues_output $OUT_PATH"eigenvalues.txt" --eigenvectors_output $OUT_PATH"eigenvectors.txt"
				mpiexec -n 1 ../pspectralclustering/kmeans --num_clusters $NUM_CLUSTERS --input $OUT_PATH"eigenvectors.txt" --output $OUT_PATH"result.txt"
				
				# Visualize partition quality
				./DrawClusters $OUT_PATH $RESOLUTION$CATEGORY $vid

				# Supertracks
				./MergeTracks $OUT_PATH
				./DrawTracks $OUT_PATH $RESOLUTION$CATEGORY $vid

				# prog(result.txt, tracks, track id) -> supertracks, edge weights (archive)
				# write supertracks to matlab
				 
			done
		done
	done
done
END_TIME=$(date +%s)
EXECUTION_TIME=$(($END_TIME - $START_TIME))
echo "Execution time: $EXECUTION_TIME seconds"

#./DominantClusterFilter $OUT_PATH $CATEGORY $vid
				#rm $OUT_PATH"sortedTrajectories.out"

				#./LocalizationScoreForVideo $OUT_PATH $RESOLUTION $CATEGORY $vid
				#./DrawClusters $OUT_PATH $RESOLUTION$CATEGORY"/" $vid


