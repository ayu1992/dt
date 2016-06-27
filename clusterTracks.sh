source ./configurations.sh
progress=0

# This script will read from Path to archive/*.out and place its
# outputs under: $_CLUSTERED_TRACKS_PATH/r={some int value}/c={some int value}/

#!/bin/bash
# Each run on this script 
# 1. reads the Dense Track features of a video
# 2. builds a graph from the tracks by treating each track as a node,
# 	 (Each _TEMPORAL_MISALIGNMENT_PENTALTY value "r" corresponds to a graph)
# 3. runs spectral clustering on the graph in hope of partitioning the graph into approximately
#    $_MAX_NUM_CLUSTER subgraphs/cluster
#
# Implementing Super Track project
# 4. then for each cluster we merge all its tracks into a super trajectory
# 5. output those trajectories and their pair-wise similarities
# 
# Implementing Chen & Corso's work
# 5. we extract the trajectories that belong to the largest cluster
# 6. dump these trajectories into archive forms under largestCluster/

make BuildGraph
make countActualClusters
make DrawTracks
make DrawClusters
make GetCoordsForClusters
make MergeTracks

for CATEGORY in "${!CATEGORIES[@]}"
do
	for ((vid=1; vid <= ${CATEGORIES[$CATEGORY]}; vid++))
	do 
		((progress++))

		# Generate a graph for each r (r is the gamma value defined in [2] page 4 equation 4)
		for r in "${_TEMPORAL_MISALIGNMENT_PENALTY[@]}"	
 		do 
			SECONDS=0
			_GRAPH_PATH=$_CLUSTERED_TRACKS_PATH"r=$r/"
			mkdir -p $_GRAPH_PATH								# No op if the folder already exists

			VIDEO_ARCHIVE=$_ARCHIVE_LOCATION$CATEGORY"_$vid.out"

			echo "Processing $VIDEO_ARCHIVE"
			
			# We can experiment with different c values on each graph
			for c in "${_MAX_NUM_CLUSTER[@]}"			
			do	
				# Output location, 
				# each combination of ($_RAW_TRACK_CAP, r, c) defines a unique output location
				OUTPUT_LOCATION=$_GRAPH_PATH"c=$c/"					
				mkdir -p $OUTPUT_LOCATION					

				# Removing any leftovers from previous runs in case states append together, 
				# but it's unlikely to happen
				rm $OUTPUT_LOCATION"result.txt"
				rm $OUTPUT_LOCATION"similarity.txt"

				./BuildGraph $VIDEO_ARCHIVE $_GRAPH_PATH $CATEGORY$vid $r $_RAW_TRACK_CAP

				echo "converting distance to similarity"
				mpiexec -n $_NUM_DISTANCE_TO_SIMILARITY_WORKERS $_DISTANCE_TO_SIMILARITY --input $_GRAPH_PATH$CATEGORY$vid"_dij.txt" --output $OUTPUT_LOCATION"similarity.txt"

				echo "Running pspectral"				
				NUM_SPACE=$(($c * 3))
				mpiexec -n $_NUM_EVD_WORKERS $_EVD --arpack_iterations 1000 --arpack_tolerance 0.000001 --eigenvalue $c --eigenspace $NUM_SPACE --input $OUTPUT_LOCATION"similarity.txt" --eigenvalues_output $OUTPUT_LOCATION"eigenvalues.txt" --eigenvectors_output $OUTPUT_LOCATION"eigenvectors.txt"
				mpiexec -n $_NUM_KMEANS_WORKERS $_KMEANS --num_clusters $c --input $OUTPUT_LOCATION"eigenvectors.txt" --output $OUTPUT_LOCATION"result.txt"
				
				# A Mapping of video name to number of non-empty clusters will be written to 
				# $OUTPUT_LOCATION/actualNumClusters
				echo "Counting actual clusters"
				./countActualClusters $OUTPUT_LOCATION $CATEGORY$vid
				
				# Uncomment to Visualize partition quality, the binaries will produce .avi files in $OUTPUT_LOCATION
				# echo "Drawing clustered results"
				#./GetCoordsForClusters $GRAPH_PATH$CATEGORY$vid $OUTPUT_LOCATION
				#./DrawClusters $OUTPUT_LOCATION $_VIDEO_LOCATION $CATEGORY $vid $c $_VIDEO_TYPE
								
				mkdir -p $OUTPUT_LOCATION"similarities/"	# Similarities between supertracks
				mkdir -p $OUTPUT_LOCATION"supertracks/"		# Here we store supertracks in txt form
				mkdir -p $OUTPUT_LOCATION"archive/"			# Here we store supertracks in archive form
				mkdir -p $OUTPUT_LOCATION"largestCluster/"
				mkdir -p $OUTPUT_LOCATION"superTrackCoords/"	# Here we store coords to plot

				# Supertracks project
				# echo "Merging trajectories"
				./MergeTracks $_GRAPH_PATH $OUTPUT_LOCATION $c $CATEGORY $vid
				
				# Largest Cluster Extraction (Chen & Corso)
				#echo "Extracting largest clusters as video representations"
				#./LargestClusterExtraction $_GRAPH_PATH $OUTPUT_LOCATION $c $CATEGORY $vid

				# Uncomment here and enable writeCoordsToFile function in MergeTracks to Visualize super tracks
				# echo "Drawing super tracks"
				#./DrawTracks $OUTPUT_LOCATION $_VIDEO_LOCATION $CATEGORY $vid $_VIDEO_TYPE
				
				duration=$SECONDS
				echo "==========================$(($duration / 60)) minutes and $(($duration % 60)) seconds=========================="
				
				pd=$(($progress * 73 / $_NUM_VIDEOS))
				printf "\r%3d.%1d%% %.${pd}s" $(( $progress * 100 / $_NUM_VIDEOS )) $(( ($progress * 1000 / $_NUM_VIDEOS) % 10 )) $bar
			done
		done
	done
done




