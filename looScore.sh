source ./configurations.sh

# Pre-generate a range of floats
gamma="$(seq 0.015625 0.1 1)"
cost="$(seq 5 0.1 1024)"

for PENALTY_R in "${_TEMPORAL_MISALIGNMENT_PENALTY_R[@]}"
do
	for NUM_CLUSTERS in "${_MAX_NUM_CLUSTER[@]}"
	do	
		./BagOfWords $_CLUSTERED_TRACKS_PATH"/r="$PENALTY_R"/c="$NUM_CLUSTERS"/archive/" $_SUPERTRACKS_PATH $CODEBOOK_SAMPLE $CODEBOOK_CENTERS 
		
		TRAININGSET_LOCATION=$_SUPERTRACKS_PATH"r="$PENALTY_R"/c="$NUM_CLUSTERS"/"
		mkdir -p $TRAININGSET_LOCATION

		TRAININGSET=$TRAININGSET_LOCATION"s="$CODEBOOK_SAMPLE",nc="$CODEBOOK_CENTERS".out"

		for g in $gamma
		do
		  for c in $cost
		  do 
		  	echo "g:"$g", c:"$c
		  	./ChiSquaredSVM $g $CODEBOOK_CENTERS $TRAININGSET $TRAININGSET_LOCATION
		  	$_SVM_TRAIN -s 0 -t 4 -c $c -v $_NUM_VIDEOS -q $TRAININGSET_LOCATION"KernelTraining.txt"
		  done	
		done
	done
done




