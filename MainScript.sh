#!/bin/bash
# This file serves as the main documentation for the whole project.
# In this project we have four components/black boxes, we build different pipelines from these boxes.
# The four components are: 
# 	1. getDenseTrackFeatures
#   2. makeArchives
#   3. clusterTracks
#   4. loo (leave-one-out)
# Each component can be activated by calling a corresponding script
# So building a pipeline is just making a combination of script calls
#
# ================================= Example 1 =================================   
# To implement the original Dense Track work [1], the workflow is:
#   Extract dense track features from each video
#   Build BagOfWords models from all trajectories
#   Calculate cross validation accuracy using a Multi-Channel Chi-Squared SVM
#
# So MainScript.sh should look like:
# ./getDenseTrackFeatures.sh
# ./makeArchives.sh
# ./looScore.sh
# Set up configurations.sh and run MainScript.sh
# ================================= Example 2 =================================   
# To implement Chen & Corso's work [2], the workflow would be:
#    Extract dense track features from each video
#    Build a graph for each video (where node distances are calculated via pixel distances and frame-misaligned penalties)
#    Run Parallel Spectral Clustering on this graph to cut it in various clusters
#    For each video, extract trajectories from the largest cluster and let them become video representation
#    Build BagOfWords models from these video representations
#    Calculate cross validation accuracy using a Multi-Channel Chi-Squared SVM
#
# MainScript.sh would look like:
# ./getDenseTrackFeatures.sh
# ./makeArchives.sh
# ./clusterTracks.sh (but comment out the ./MergeTrack command; uncomment the ./LargestClusterExtraction command, see documentation inside superTracks.sh)
# ./looScore.sh
# Set up configurations.sh and run MainScript.sh
# ================================= Example 3 =================================   
# To generate super trajectories: 
# ./getDenseTrackFeatures.sh
# ./makeArchives.sh
# ./clusterTracks.sh (but comment out the ./LargestClusterExtraction command; uncomment the ./MergeTrack command, see documentation inside superTracks.sh)
# ./looScore.sh
# Set up configurations.sh and run MainScript.sh

make all 

echo $w"Extracting Dense Track"$w
# getVideos.sh extracts Dense Tracks from videos, output .features files in the same location (next to their video files)
# NOTE: check and make sure your videos files are organized in a similar manner like the following: 
# (Suppose we're running our code on a dataset called UCFSports,
#  videos have been sliced into Training and Testing piles, 
#  there are 3 action types in total : jump, sleep and dive
#  there are several videos under each action type)
# NOTE: In the diagram below, a back-slash behind a variable name indicates that it is a folder name
#       "dt/" is a folder named "dt"
#
# 								dt/
# 								/
#				Dataset name/(ex. UCFSports/)			# Dataset name will be the value for _DATASET in configurations.sh (line 9)
# 					/ 				\
# 				Training/			Testing/
#			   /    |    \			/    |     \
#			jump/ sleep/ dive/ 	 jump/ sleep/ dive/
#           / 		| \      \		 |    |      | \       
# 		 1.avi	 1.avi 2.avi  1.avi	 ... ...   1.avi 2.avi 
#
# If your dataset has Training and Testing sets, 
#   Set up configuration.sh to one of the sets
#	Run the pipeline (anything except loo)
#   Set up configuration.sh to the other set
#   Run the pipeline (anything except loo)
#   In BagOfWords.cpp and ChiSquaredSVM.cpp, uncomment the line at the top of the files "#define TESTSUPPORT"
#   >> make BagOfWords
#   >> make ChiSquaredSVM
#   Modify looScore.sh line 33. Add support to let it read from test set features generated from ChiSquaredSVM.cpp and 
#   Change libsvm command from cross validation to prediction.
#   >> ./looScore.sh 
#
# The following command activates the getDenseTrackFeatures component
# you can type it in the terminal too : >> ./getDenseTrackFeatures.sh
./getDenseTrackFeatures.sh	

# Pack dense track features into boost::archives, place archives in a new folder	
# Turning all of our data (dense tracks) into archive forms makes them 
# easier to access (time efficient) and process (object concept) for future purposes
# echo $w"Packing .features into archives"$w
./makeArchives.sh

# echo $w"Build Graph, run spectral and merge trajectories into super tracks"$w
./clusterTracks.sh

# echo $w"Leave one out cross validation scores"$w
./looScore.sh

# Literature references
# [1] https://lear.inrialpes.fr/people/wang/dense_trajectories
# [2] http://web.eecs.umich.edu/~jjcorso/pubs/jcorso_ICCV2015_implicitmotion.pdf