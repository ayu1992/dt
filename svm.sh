#!/bin/bash
# Read from TrajectoryDump.data
# Compute codebook
# Do RBF-SVM

NUM_CLUSTERS=1
PARAM_R=3

# Output path
OUT_PATH="ClusteredTrajectories/r="
OUT_PATH+=$PARAM_R
OUT_PATH+="/c="
OUT_PATH+=$NUM_CLUSTERS
OUT_PATH+="/"

make ConstructCodebook

#./ConstructCodebook $OUT_PATH

../libsvm-3.21/svm-train -s 0 -t 0 $OUT_PATH"TrainingSet.data"
../libsvm-3.21/svm-predict  $OUT_PATH"TestSet.data" $OUT_PATH"TrainingSet.data.model" $OUT_PATH"result.txt"