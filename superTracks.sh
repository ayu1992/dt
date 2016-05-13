DATASET="UCFSports/"
declare -A CATEGORIES
#['Riding-Horse']=12 ['Walk-Front']=22 ['Run-Side']=13
#CATEGORIES=(['Run-Side']=13)
#CATEGORIES=(['Golf-Swing-Back']=5 ['Golf-Swing-Front']=8 ['SkateBoarding-Front']=12 ['Swing-Bench']=20)
#CATEGORIES=(['Diving-Side']=14 ['Kicking-Front']=10 ['Kicking-Side']=10)
#CATEGORIES=(['Lifting']=6 ['Golf-Swing-Side']=5 ['Swing-SideAngle']=13 )
CATEGORIES=(['Lifting']=6 ['Golf-Swing-Side']=5 ['Swing-SideAngle']=13 ['Diving-Side']=14 ['Kicking-Front']=10 ['Kicking-Side']=10 ['Golf-Swing-Back']=5 ['Golf-Swing-Front']=8 ['SkateBoarding-Front']=12 ['Swing-Bench']=20 ['Riding-Horse']=12 ['Walk-Front']=22 ['Run-Side']=13)
RESOLUTION=$DATASET"original/"
EXTRACTION="idt/"
PROCESS=1

#rm BuildGraph
#rm MergeTracks
#rm DrawTracks
#rm GetCoordsForClusters
#rm DrawClusters
#rm countActualClusters
make BuildGraph
make MergeTracks
make DrawTracks
make GetCoordsForClusters
make DrawClusters
make countActualClusters
#make LocalizationScoreForVideo
#make DominantClusterFilter

numVideos=150
progress=0
bar="[=======================================================================]"

for CATEGORY in "${!CATEGORIES[@]}"			# '!' expands keys, no '!' expands values
do
	for ((vid=1; vid <= ${CATEGORIES[$CATEGORY]}; vid++))
	do 
		((progress++))
		for PARAM_R in 0.05 #0.1
		do 
			SECONDS=0
			#START_TIME=$(date +%s)
			GRAPH_PATH="ClusteredTrajectories/r="$PARAM_R"/"
			mkdir -p $GRAPH_PATH								# No op if the folder already exists

			VIDEO_NAME=$RESOLUTION$CATEGORY"/"$EXTRACTION$vid".features"
			
			echo "Processing "$VIDEO_NAME
#			rm $GRAPH_PATH"dij.txt"
#			rm $GRAPH_PATH"sortedTrajectories.out"
			
			for NUM_CLUSTERS in 500
			do	
				# Output location
				OUT_PATH=$GRAPH_PATH"c="$NUM_CLUSTERS"/p"$PROCESS"/"					
				mkdir -p $OUT_PATH								# No op if the folder already exists

				rm $OUT_PATH"result.txt"
				rm $OUT_PATH"similarity.txt"

				./BuildGraph $VIDEO_NAME $GRAPH_PATH $CATEGORY$vid $PARAM_R
				echo "converting distance to similarity"
				mpiexec -n 8 ../pspectralclustering/distance_to_similarity --input $GRAPH_PATH$CATEGORY$vid"_dij.txt" --output $OUT_PATH"similarity.txt"


				# wipe out intermediate data, leave a clean state for this run
				#rm $OUT_PATH"*.txt"

				#rm $OUT_PATH"*.out"

				echo "Running pspectral"				
				NUM_SPACE=$(($NUM_CLUSTERS * 3))
				mpiexec -n 10 ../pspectralclustering/evd --arpack_iterations 1000 --arpack_tolerance 0.000001 --eigenvalue $NUM_CLUSTERS --eigenspace $NUM_SPACE --input $OUT_PATH"similarity.txt" --eigenvalues_output $OUT_PATH"eigenvalues.txt" --eigenvectors_output $OUT_PATH"eigenvectors.txt"
				mpiexec -n 4 ../pspectralclustering/kmeans --num_clusters $NUM_CLUSTERS --input $OUT_PATH"eigenvectors.txt" --output $OUT_PATH"result.txt"
				
				echo "Counting actual clusters"
				./countActualClusters $OUT_PATH $CATEGORY$vid
				
				# Visualize partition quality
				#echo "Drawing clustered results"
				#./GetCoordsForClusters $GRAPH_PATH$CATEGORY$vid $OUT_PATH
				#./DrawClusters $OUT_PATH $RESOLUTION$CATEGORY"/" $CATEGORY $vid $NUM_CLUSTERS
								
				# Supertracks
				echo "Merging trajectories"
				./MergeTracks $GRAPH_PATH $OUT_PATH $NUM_CLUSTERS $CATEGORY $vid
								
				# Visualize super tracks
				#echo "Drawing super tracks"
				#./DrawTracks $OUT_PATH $RESOLUTION$CATEGORY"/" $CATEGORY $vid
				
				#END_TIME=$(date +%s)
				#EXECUTION_TIME=$(($END_TIME - $START_TIME))
				duration=$SECONDS
				echo "==========================$(($duration / 60)) minutes and $(($duration % 60)) seconds=========================="
				
				pd=$(($progress * 73 / $numVideos))
				printf "\r%3d.%1d%% %.${pd}s" $(( $progress * 100 / $numVideos )) $(( ($progress * 1000 / $numVideos) % 10 )) $bar
			done
		done
	done
done




