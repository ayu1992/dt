#GAMMA=1.0
#C_VAL=1.0
echo "Enter Gamma : "
read GAMMA
echo "Enter c : "
read C_VAL

TRAINING_SET_PATH="NoClustering/Features/HoG/KernelTraining.txt"
TEST_SET_PATH="NoClustering/Features/Displacements/KernelTest.txt"
rm $TRAINING_SET_PATH
rm $TEST_SET_PATH

make ChiSquaredSVM
./ChiSquaredSVM $GAMMA

# Uncomment for real prediction accurarcy
#../libsvm/svm-train -s 0 -t 4 -c $C_VAL $TRAINING_SET_PATH kernel.model
#../libsvm/svm-predict $TEST_SET_PATH kernel.model result.out

# Leave one out / 108 fold cross validation
../libsvm/svm-train -s 0 -t 4 -c $C_VAL -v 133 $TRAINING_SET_PATH

