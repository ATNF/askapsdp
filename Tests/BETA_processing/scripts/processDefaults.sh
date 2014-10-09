#!/usr/bin/env bash
#
# This file takes the default values, after any modification by a
# user's config file, and creates other variables that depend upon
# them and do not require user input.
#
# (c) Matthew Whiting, ATNF, 2014


####################
# Check for the presence of simager. If not available, turn off
# spectral-line imaging

if [ $doSpectralImaging == true ] && [ ! -e $simager ]; then
    echo "WARNING! Spectral-line imager '${simager}' not found. Setting doSpectralImaging=false."
    doSpectralImaging=false
fi


####################
# Input Measurement Sets
#  We define these based on the SB number

# 1934-638 calibration
if [ "$input1934" == "" ]; then
    if [ $SB1934 != "SET_THIS" ]; then
	sb1934dir=$SBdir/$SB1934
	if [ `\ls $sb1934dir | grep "ms" | wc -l` == 1 ]; then
	    input1934=$sb1934dir/`\ls $sb1934dir | grep "ms"`
	else
	    echo "SB directory $SB1934 has more than one measurement set. Please specify with variable 'input1934'."
	fi
    else
	echo "You must set either SB1934 (scheduling block number) or input1934 (1934 measurement set)."
    fi
fi
if [ "$input1934" == "" ]; then
    if [ $do1934cal == true ]; then
	echo "input1934 not defined. Turning off 1934-638 processing with do1934cal=false."
    fi
    do1934cal=false
fi

# science observation
if [ "$inputSci" == "" ]; then
    if [ $SBscience != "SET_THIS" ]; then
	sbScienceDir=$SBdir/$SBscience
	if [ `\ls $sbScienceDir | grep "ms" | wc -l` == 1 ]; then
	    inputSci=$sbScienceDir/`\ls $sbScienceDir | grep "ms"`
	else
	    echo "SB directory $SBscience has more than one measurement set. Please specify with variable 'inputSci'."
	fi
    else
	echo "You must set either SBscience (scheduling block number) or inputSci (Science observation measurement set)."
    fi
fi
if [ "$inputSci" == "" ]; then
    if [ $doSci == true ]; then
	echo "inputSci not defined. Turning off science processing with doSci=false."
    fi
    doSci=false
fi

####################
# Check the number of beams

requestedBeams=`echo $BEAM_MAX $BEAM_MIN | awk '{print $1-$2+1}'`
if [ $nbeam -gt $requestedBeams ]; then
    nbeam=$requestedBeams
fi

####################
# Parameters required for continuum imaging

# nchanContSci = number of channels after averaging
nchanContSci=`echo $nchanSci $chanAverageSci | awk '{print $1/$2}'`

# nworkergroupsSci = number of worker groups, used for MFS imaging. 
nworkergroupsSci=`echo $ntermsSci | awk '{print 2*$1-1}'`

# total number of CPUs required for MFS continuum imaging, including
# the master
NUM_CPUS_CONTIMG_SCI=`echo $nchanContSci $nworkergroupsSci | awk '{print $1*$2+1}'`


####################
# Define the beam arrangements for linmos
. ${SCRIPTDIR}/beamArrangements.sh

# Fix the direction string for linmos - don't need the J2000 bit
linmosFeedCentre=`echo $directionSci | awk -F',' '{printf "%s,%s]",$1,$2}'`
