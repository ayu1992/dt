#!/bin/bash

#folder=8

WIDTH=720
HEIGHT=404
nw=360
nh=202

for folder in 13
do 
	FILEPATH=Run-Side/0
	FILEPATH+=$folder
	FILEPATH+="/"	
	
	cd $FILEPATH

	i=0

	for file in *.jpg
	do
		newname=$i
		newname+=".jpg"
		mv "$file" "$newname"
		i=$((i+1))
	done

	avconv -f image2 -i %d.jpg -r 10 -s "$WIDTH"x"$HEIGHT" temp.avi
	avconv -i temp.avi -s "$nw"x"$nh" "$folder".avi

	cp "$folder".avi ../../../../dense_trajectory/InputVideos/Running/"$folder".avi

	cd ../../
done