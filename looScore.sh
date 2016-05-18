source ./configurations.sh

# Pre-generate a range of floats
gamma="$(seq 0.015625 0.1 1)"
cost="$(seq 5 0.1 1024)"

R=0.025								# todo: make this a range of gammas, specified in the configs file

PATH="SuperTracks/sampleCut="$RAW_TRACK_CAP"/"$CHANNEL"/"

./BagOfWords "ClusteredTrajectories/sample="$RAW_TRACK_CAP"/r="$R"/c=500/" $PATH $CODEBOOK_SAMPLE $CODEBOOK_CENTERS 

TRAININGSET="s="$CODEBOOK_SAMPLE",nc="$CODEBOOK_CENTERS".out"

for g in $gamma
do
  for c in $cost
  do 
  	echo "g:"$g", c:"$c
  	./ChiSquaredSVM $g $CODEBOOK_CENTERS $PATH$TRAININGSET $PATH
  	../libsvm/svm-train -s 0 -t 4 -c $c -v 150 -q $PATH"KernelTraining.txt"
  done	
done

