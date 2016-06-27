#!/bin/bash
# Executes Leave one out cross validation
source /home/hydralisk/Documents/dt/configurations.sh

# Pre-generate a range of floats
gamma="$(seq 0.015625 0.1 1)"
cost="$(seq 8 0.5 1024)"

make ChiSquaredSVM
make BagOfWords

for r in "${_TEMPORAL_MISALIGNMENT_PENALTY[@]}"
do
	for c in "${_MAX_NUM_CLUSTER[@]}"
	do	
		
		_TRAININGSET_DESTINATION=$_TRAININGSET_DESTINATION"r=$r/c=$c/"
		echo $TRAININGSET_DESTINATION

		mkdir -p $_TRAININGSET_DESTINATION
			
		# For super track, uncomment the next line
		# _TRAINING_TRAJECTORY_LOCATION=$_TRAINING_TRAJECTORY_LOCATION"r=$r/c=$c/archive/"		
		# For [2], uncomment the next line
		_TRAINING_TRAJECTORY_LOCATION=$_TRAINING_TRAJECTORY_LOCATION"r=$r/c=$c/largestCluster/"
		# For [1]
		# just comment out the aforementioned two _TRAINING_TRAJECTORY_LOCATION definitions. _TRAINING_TRAJECTORY_LOCATION doesn't need to be updated in this file.

		./BagOfWords $_TRAINING_TRAJECTORY_LOCATION $_TRAININGSET_DESTINATION $_CODEBOOK_SAMPLE $_CODEBOOK_CENTERS 
		
		# For test set support, comment the previous call to BagOfWords and use this instead :
		# ./BagOfWords $_TRAJECTORY_LOCATION $_TRAININGSET_DESTINATION $_CODEBOOK_SAMPLE $_CODEBOOK_CENTERS $_TESTING_TRAJECTORY_LOCATION
		# TESTINGSET=$_TESTING_TRAJECTORY_LOCATION"test.out"
		
		TRAININGSET=$_TRAININGSET_DESTINATION"s=$_CODEBOOK_SAMPLE,nc=$_CODEBOOK_CENTERS.out"

		for g in $gamma
		do
		  for c in $cost
		  do 
		  	echo "g:"$g", c:"$c
		  	./ChiSquaredSVM $g $_CODEBOOK_CENTERS $TRAININGSET $_TRAININGSET_DESTINATION
		  	# For test set support, run this instead
		  	# ./ChiSquaredSVM $g $_CODEBOOK_CENTERS $TRAININGSET $_TRAININGSET_DESTINATION $_TESTING_TRAJECTORY_LOCATION
		  	$_SVM_TRAIN -s 0 -t 4 -c $c -v $_NUM_VIDEOS -q $_TRAININGSET_DESTINATION"KernelTraining.txt"
		  done	
		done
	done
done




