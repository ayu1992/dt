source ./configurations.sh
progress=0

for CATEGORY in "${!CATEGORIES[@]}"			# '!' expands keys, no '!' expands values
do
	for ((vid=1; vid <= ${CATEGORIES[$CATEGORY]}; vid++))
	do 
		((progress++))
		for PENALTY_R in "${_TEMPORAL_MISALIGNMENT_PENALTY_R[@]}"
 		do 
			SECONDS=0
			_GRAPH_PATH=$_CLUSTERED_TRACKS_PATH"r="$PENALTY_R"/"
			mkdir -p $GRAPH_PATH								# No op if the folder already exists

			VIDEO_NAME=$_ARCHIVE_LOCATION"/"$vid".out"
			
			echo "Processing "$VIDEO_NAME
			
			for NUM_CLUSTERS in "${_MAX_NUM_CLUSTER[@]}"
			do	
				# Output location
				OUTPUT_LOCATION=$GRAPH_PATH"c="$NUM_CLUSTERS"/"					
				mkdir -p $OUTPUT_LOCATION								# No op if the folder already exists

				rm $OUTPUT_LOCATION"result.txt"
				rm $OUTPUT_LOCATION"similarity.txt"

				./BuildGraph $VIDEO_NAME $GRAPH_PATH $CATEGORY$vid $PENALTY_R

				echo "converting distance to similarity"
				mpiexec -n $_NUM_DISTANCE_TO_SIMILARITY_WORKERS $_DISTANCE_TO_SIMILARITY --input $GRAPH_PATH$CATEGORY$vid"_dij.txt" --output $OUTPUT_LOCATION"similarity.txt"

				echo "Running pspectral"				
				NUM_SPACE=$(($NUM_CLUSTERS * 3))
				mpiexec -n $_NUM_EVD_WORKERS $_EVD --arpack_iterations 1000 --arpack_tolerance 0.000001 --eigenvalue $NUM_CLUSTERS --eigenspace $NUM_SPACE --input $OUTPUT_LOCATION"similarity.txt" --eigenvalues_output $OUTPUT_LOCATION"eigenvalues.txt" --eigenvectors_output $OUTPUT_LOCATION"eigenvectors.txt"
				mpiexec -n $_NUM_KMEANS_WORKERS $_KMEANS --num_clusters $NUM_CLUSTERS --input $OUTPUT_LOCATION"eigenvectors.txt" --output $OUTPUT_LOCATION"result.txt"
				
				echo "Counting actual clusters"
				./countActualClusters $OUTPUT_LOCATION $CATEGORY$vid
				
				# Visualize partition quality
				#echo "Drawing clustered results"
				#./GetCoordsForClusters $GRAPH_PATH$CATEGORY$vid $OUTPUT_LOCATION
				#./DrawClusters $OUTPUT_LOCATION $RESOLUTION$CATEGORY"/" $CATEGORY $vid $NUM_CLUSTERS
								
				# Supertracks
				echo "Merging trajectories"
				./MergeTracks $GRAPH_PATH $OUTPUT_LOCATION $NUM_CLUSTERS $CATEGORY $vid
								
				# Visualize super tracks
				#echo "Drawing super tracks"
				#./DrawTracks $OUTPUT_LOCATION $RESOLUTION$CATEGORY"/" $CATEGORY $vid
				
				duration=$SECONDS
				echo "==========================$(($duration / 60)) minutes and $(($duration % 60)) seconds=========================="
				
				pd=$(($progress * 73 / $numVideos))
				printf "\r%3d.%1d%% %.${pd}s" $(( $progress * 100 / $numVideos )) $(( ($progress * 1000 / $numVideos) % 10 )) $bar
			done
		done
	done
done




