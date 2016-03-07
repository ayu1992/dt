#!/bin/bash
# Read from TrajectoryDump.data
# Compute codebook
# Do RBF-SVM

NUM_CLUSTERS=1
PARAM_R=50

# Output path
OUT_PATH="ClusteredTrajectories/r="
OUT_PATH+=$PARAM_R
OUT_PATH+="/c="
OUT_PATH+=$NUM_CLUSTERS
OUT_PATH+="/"

#make showTrajectories
#./showTrajectories

START_TIME=$(date +%s)
make ConstructCodebook

rm $OUT_PATH"TrainingSet.data"
rm $OUT_PATH"TestingSet.data"

./ConstructCodebook $OUT_PATH
END_TIME=$(date +%s)
EXECUTION_TIME=$(($END_TIME - $START_TIME))
echo "Execution time: $EXECUTION_TIME seconds"

python ../libsvm-3.21/tools/grid.py -log2c -5,5,1 -log2g -5,5,1 -v 5 $OUT_PATH"TrainingSet.data"

#../libsvm-3/svm-scale -l 0 -u 1 $OUT_PATH"TrainingSet.data"
	# g:5~0.5
#../libsvm-3.21/svm-train -s 0 -c 250 -t 2 -g 5 -e 0.001 $OUT_PATH"TrainingSet.data" $OUT_PATH"svm.model"
#../libsvm-3.21/svm-predict $OUT_PATH"TestingSet.data" $OUT_PATH"svm.model" $OUT_PATH"result.txt"	
