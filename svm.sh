

#MIN_GAMMA=$((2^-10))
#MAX_GAMMA=1
make ChiSquaredSVM

gamma="$(seq 0.0009765625 0.1 1)"
cost="$(seq 2 0.1 1024)"

SAMPLE=8000
CHANNEL=All
CODEBOOK_SAMPLE=5000
CODEBOOK_CENTERS=500

PATH="SuperTracks/sampleCut="$SAMPLE"/"$CHANNEL"/"
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

