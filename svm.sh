#!/bin/bash
# Read from TrajectoryDump.data
# Compute codebook
# Do RBF-SVM

NUM_CLUSTERS=2
PARAM_R=20

# Output path
OUT_PATH="ClusteredTrajectories/r="
OUT_PATH+=$PARAM_R
OUT_PATH+="/c="
OUT_PATH+=$NUM_CLUSTERS
OUT_PATH+="/Medium/"

#make showTrajectories
#./showTrajectories

make ConstructCodebook
START_TIME=$(date +%s)
./ConstructCodebook $OUT_PATH
END_TIME=$(date +%s)
EXECUTION_TIME=$(($END_TIME - $START_TIME))
echo "Execution time: $EXECUTION_TIME seconds"

for fold in 0 1 2 3 4
do
	../libsvm-3/svm-scale -l 0 -u 1 $OUT_PATH"TrainingSet"$fold".data"
	../libsvm-3/svm-scale -l 0 -u 1 $OUT_PATH"ValidationSet"$fold".data"
	# g:5~0.5
	../libsvm-3.21/svm-train -s 0 -c 250 -t 2 -g 5 -e 0.0001 $OUT_PATH"TrainingSet"$fold".data"
	../libsvm-3.21/svm-predict $OUT_PATH"ValidationSet"$fold".data" "TrainingSet"$fold".data.model" $OUT_PATH"result"$fold".txt"	
	#../libsvm-3.21/svm-predict  $OUT_PATH"TestingSet.data" "TrainingSet"$fold".data.model" $OUT_PATH"result"$fold".txt"	
done