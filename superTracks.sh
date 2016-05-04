DATASET="UCFSports/"
declare -A CATEGORIES
#CATEGORIES=(['Diving-Side']=14 ['Golf-Swing-Back']=5 ['Golf-Swing-Side']=5 ['Kicking-Front']=10 ['Kicking-Side']=10 ['Lifting']=6 ['Riding-Horse']=12 ['Run-Side']=13 ['SkateBoarding-Front']=12 ['Swing-Bench']=20 ['Swing-SideAngle']=13 ['Walk-Front']=22)
CATEGORIES=(['Diving-Side']=1)
RESOLUTION=$DATASET"original/"

rm BuildGraph
rm MergeTracks
rm DrawTracks
rm GetCoordsForClusters
rm DrawClusters

make BuildGraph
make MergeTracks
make DrawTracks
make GetCoordsForClusters
make DrawClusters

#make LocalizationScoreForVideo
#make DominantClusterFilter

START_TIME=$(date +%s)

for CATEGORY in "${!CATEGORIES[@]}"			# '!' expands keys, no '!' expands values
do
	for ((vid=1; vid <= ${CATEGORIES[$CATEGORY]}; vid++))
	do 
		for PARAM_R in 1
		do 
			GRAPH_PATH="ClusteredTrajectories/r="$PARAM_R"/"
			mkdir -p $GRAPH_PATH								# No op if the folder already exists

			VIDEO_NAME=$RESOLUTION$CATEGORY"/"$vid".features"
			
			echo "Processing "$VIDEO_NAME
			rm $GRAPH_PATH"dij.txt"
			rm $GRAPH_PATH"sortedTrajectories.out"

			./BuildGraph $VIDEO_NAME $GRAPH_PATH $PARAM_R
			mpiexec -n 2 ../pspectralclustering/distance_to_similarity --input $GRAPH_PATH"dij.txt" --output $GRAPH_PATH"similarity.txt"

			for NUM_CLUSTERS in 5
			do
				# Output location
				OUT_PATH=$GRAPH_PATH"c="$NUM_CLUSTERS"/"					
				mkdir -p $OUT_PATH								# No op if the folder already exists

				# wipe out intermediate data, leave a clean state for this run
				rm $OUT_PATH"*.txt"
				rm $OUT_PATH"*.out"

				echo "Running pspectral"				
				NUM_SPACE=$(($NUM_CLUSTERS * 3))
				mpiexec -n 10 ../pspectralclustering/evd --arpack_iterations 1000 --arpack_tolerance 0.000001 --eigenvalue $NUM_CLUSTERS --eigenspace $NUM_SPACE --input $GRAPH_PATH"similarity.txt" --eigenvalues_output $OUT_PATH"eigenvalues.txt" --eigenvectors_output $OUT_PATH"eigenvectors.txt"
				mpiexec -n 4 ../pspectralclustering/kmeans --num_clusters $NUM_CLUSTERS --input $OUT_PATH"eigenvectors.txt" --output $OUT_PATH"result.txt"
							
				./GetCoordsForClusters $GRAPH_PATH $OUT_PATH

				# Visualize partition quality
				./DrawClusters $OUT_PATH $RESOLUTION$CATEGORY"/" $CATEGORY $vid $NUM_CLUSTERS
								
				# Supertracks
				./MergeTracks $GRAPH_PATH $OUT_PATH $NUM_CLUSTERS $CATEGORY$vid
								
				# Visualize super tracks
				./DrawTracks $OUT_PATH $RESOLUTION$CATEGORY"/" $CATEGORY $vid
			done
		done
	done
done

END_TIME=$(date +%s)
EXECUTION_TIME=$(($END_TIME - $START_TIME))
echo "Execution time: $EXECUTION_TIME seconds"


