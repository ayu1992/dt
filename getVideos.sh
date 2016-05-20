#!/bin/bash
source ./configurations.sh

# Regardless of the dataset's folder/file naming convention, this process should output dense track 
# features in this format: $_VIDEO_LOCATION$CATEGORY/{integer vid}.features
# E.g.: Path to dataset/Diving-Side/10.features
#		Path to dataset/Diving-Side/2.features
#       Path to dataset/Lifting/1.features


# TODO: make this take a flag: idt/dt

for CATEGORY in "${!CATEGORIES[@]}"
do
	for vid in $(seq -f %03g ${CATEGORIES[$CATEGORY]})		# generates vid : {001, 002, ... 010 ...}
	do
		echo "Processing $CATEGORY $vid"

		# strip away leading zeros
		vidWithoutLeadingZeros=$(echo $vid | sed 's/^0*//')

		# The following two VIDEO_NAME variables are examples to handle other filename formats
		# VIDEO_NAME="$_VIDEO_LOCATION$CATEGORY/${CATEGORY//-/_}_$vid$_VIDEO_TYPE"	# Kicking-Front/Kicking_Front_003.vob
		# VIDEO_NAME="$_VIDEO_LOCATION$CATEGORY$vid$_VIDEO_TYPE"			# Kicking-Front/012.vob
		VIDEO_NAME="$_VIDEO_LOCATION$CATEGORY/$vidWithoutLeadingZeros$_VIDEO_TYPE"

		$_PATH_TO_IDT_BINARY $VIDEO_NAME > "$_VIDEO_LOCATION$CATEGORY/$vidWithoutLeadingZeros.features" 
		
		# Similar for the flipped versions
		# $_PATH_TO_IDT_BINARY "$_VIDEO_LOCATION$CATEGORY/${CATEGORY//-/_}_$vid_flipped$_VIDEO_TYPE" > "$_VIDEO_LOCATION$CATEGORY/$vid_flipped.features" 

	done
done
