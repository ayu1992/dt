#!/bin/bash
source ./configurations.sh

for CATEGORY in "${!CATEGORIES[@]}"
do
	for vid in $(seq -f %03g ${CATEGORIES[$CATEGORY]})
	do
		echo "Processing "$CATEGORY" "$vid
		# TODO: make this take a flag: idt/dt
		# This process also renames the videos files from "003.vob" to "3.features"
		$_PATH_TO_IDT_BINARY > $_VIDEO_LOCATION$CATEGORY"/${CATEGORY//-/_}_"$vid$_VIDEO_TYPE > $_VIDEO_LOCATION$CATEGORY"/"$vid".features" 
		$_PATH_TO_IDT_BINARY > $_VIDEO_LOCATION$CATEGORY"/${CATEGORY//-/_}_"$vid"_flipped"$_VIDEO_TYPE > $_VIDEO_LOCATION$CATEGORY"/"$vid"_flipped.features" 
		
		#./DenseTrack "UCFSports/original/"$CATEGORY"/"$vid".vob" > "UCFSports/original/"$CATEGORY"/"$vid".features" 
#		name="UCFSports/original/"$CATEGORY"/"${CATEGORY//-/_}"_"$vid"_flipped.vob"
#		echo $name
#		./DenseTrack $name > "UCFSports/original/"$CATEGORY"/"$vid"_flipped.features"
	done
done
