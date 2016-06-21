#!/bin/bash
source /home/hydralisk/Documents/dt/configurations.sh

# Pre-generate a range of floats
gamma="$(seq 0.015625 0.1 1)"
cost="$(seq 5 0.1 1024)"

for r in "${_TEMPORAL_MISALIGNMENT_PENALTY[@]}"
do
	for c in "${_MAX_NUM_CLUSTER[@]}"
	do	
		echo $_CLUSTERED_TRACKS_PATH"r=$r/c=$c/archive/"

		TRAININGSET_LOCATION=$_SUPERTRACKS_PATH"r=$r/c=$c/"
		mkdir -p $TRAININGSET_LOCATION

		./BagOfWords $_CLUSTERED_TRACKS_PATH"r=$r/c=$c/largestCluster/" $TRAININGSET_LOCATION $_CODEBOOK_SAMPLE $_CODEBOOK_CENTERS 

		TRAININGSET=$TRAININGSET_LOCATION"s=$_CODEBOOK_SAMPLE,nc=$_CODEBOOK_CENTERS.out"

		for g in $gamma
		do
		  for c in $cost
		  do 
		  	echo "g:"$g", c:"$c
		  	./ChiSquaredSVM $g $_CODEBOOK_CENTERS $TRAININGSET $TRAININGSET_LOCATION
		  	$_SVM_TRAIN -s 0 -t 4 -c $c -v $_NUM_VIDEOS -q $TRAININGSET_LOCATION"KernelTraining.txt"
		  done	
		done
	done
done




