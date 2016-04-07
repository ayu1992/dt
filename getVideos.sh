#!/bin/bash
declare -A CATEGORIES
CATEGORIES=( ['Diving-Side']=14 ['Golf-Swing-Back']=5 ['Golf-Swing-Front']=8 ['Golf-Swing-Side']=5 ['Kicking-Front']=10 ['Kicking-Side']=10 ['Lifting']=6 ['Riding-Horse']=12 ['Run-Side']=13 ['SkateBoarding-Front']=12 ['Swing-Bench']=20 ['Swing-SideAngle']=13 ['Walk-Front']=22)
#CATEGORIES=(['Swing-SideAngle']=6)
for CATEGORY in "${!CATEGORIES[@]}"			# '!' expands keys, no '!' expands values
do
	for (( vid=1; vid <= ${CATEGORIES[$CATEGORY]}; vid++))
	do
		echo $CATEGORY
		echo $vid
		../improved_trajectory_release/release/DenseTrackStab "UCFSports/original/"$CATEGORY"/"$vid".avi" > "UCFSports/original/"$CATEGORY"/"$vid".features" 
	done
done
