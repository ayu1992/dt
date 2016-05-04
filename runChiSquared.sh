echo "Enter num videos : "
read NUM_VIDEOS
#### Grid search to find best params ####
#/usr/local/MATLAB/R2015b/bin/matlab -nojvm
#ChiSquaredGrid('NoClustering/Features/earlyDS/TrainingSet.out', $NUM_VIDEOS)


echo "Enter Gamma : "
read GAMMA
echo "Enter c : "
read C_VAL

TRAINING_SET_PATH="NoClustering/Features/HoG/KernelTraining.txt"
#TEST_SET_PATH="NoClustering/Features/Displacements/KernelTest.txt"

touch $TRAINING_SET_PATH
#rm $TEST_SET_PATH

	rm $TRAINING_SET_PATH
	make ChiSquaredSVM
	./ChiSquaredSVM $GAMMA

	# Leave one out / 108 fold cross validation Or prediction
	../libsvm/svm-train -s 0 -t 4 -c $C_VAL -v $NUM_VIDEOS -q $TRAINING_SET_PATH

# Uncomment for real prediction accurarcy
#../libsvm/svm-train -s 0 -t 4 -c $C_VAL $TRAINING_SET_PATH kernel.model
#../libsvm/svm-predict $TEST_SET_PATH kernel.model result.out

