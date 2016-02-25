#!/bin/bash
# Read from TrajectoryDump.data
# Compute codebook
# Do RBF-SVM

NUM_CLUSTERS=5
PARAM_R=3

# Output path
OUT_PATH="ClusteredTrajectories/r="
OUT_PATH+=$PARAM_R
OUT_PATH+="/c="
OUT_PATH+=$NUM_CLUSTERS
OUT_PATH+="/"

#make showTrajectories
#./showTrajectories
rm $OUT_PATH"TestSet.data"
rm $OUT_PATH"TrainingSet.data"
rm $OUT_PATH"result.txt"


make ConstructCodebook
START_TIME=$(date +%s)
./ConstructCodebook $OUT_PATH
END_TIME=$(date +%s)
EXECUTION_TIME=$(($END_TIME - $START_TIME))
echo "Execution time: $EXECUTION_TIME seconds"

# Must be executed from $OUT_PATH somehow
../libsvm-3.21/svm-train -s 0 -c 0.5 -t 2 -g 0.5 -e 0.1 $OUT_PATH"TrainingSet.data"
../libsvm-3.21/svm-predict  $OUT_PATH"TestSet.data" "TrainingSet.data.model" $OUT_PATH"result.txt"