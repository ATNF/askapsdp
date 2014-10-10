#!/usr/bin/env bash
#
# This file takes the default values, after any modification by a
# user's config file, and creates other variables that depend upon
# them and do not require user input.
#
# (c) Matthew Whiting, CSIRO ATNF, 2014

####################
####################
# Check for the presence of simager. If not available, turn off
# spectral-line imaging

if [ $DO_SPECTRAL_IMAGING == true ] && [ ! -e $simager ]; then
    echo "WARNING! Spectral-line imager '${simager}' not found. Setting DO_SPECTRAL_IMAGING=false."
    DO_SPECTRAL_IMAGING=false
fi


####################
# Input Measurement Sets
#  We define these based on the SB number

# 1934-638 calibration
if [ "$MS_INPUT_1934" == "" ]; then
    if [ $SB_1934 != "SET_THIS" ]; then
	sb1934dir=$DIR_SB/$SB_1934
	if [ `\ls $sb1934dir | grep "ms" | wc -l` == 1 ]; then
	    MS_INPUT_1934=$sb1934dir/`\ls $sb1934dir | grep "ms"`
	else
	    echo "SB directory $SB_1934 has more than one measurement set. Please specify with parameter 'MS_INPUT_1934'."
	fi
    else
	echo "You must set either 'SB_1934' (scheduling block number) or 'MS_INPUT_1934' (1934 measurement set)."
    fi
fi
if [ "$MS_INPUT_1934" == "" ]; then
    if [ $DO_1934_CAL == true ]; then
	echo "Parameter 'MS_INPUT_1934' not defined. Turning off 1934-638 processing with DO_1934_CAL=false."
    fi
    DO_1934_CAL=false
fi

# science observation
if [ "$MS_INPUT_SCIENCE" == "" ]; then
    if [ $SB_SCIENCE != "SET_THIS" ]; then
	sbScienceDir=$DIR_SB/$SB_SCIENCE
	if [ `\ls $sbScienceDir | grep "ms" | wc -l` == 1 ]; then
	    MS_INPUT_SCIENCE=$sbScienceDir/`\ls $sbScienceDir | grep "ms"`
	else
	    echo "SB directory $SB_SCIENCE has more than one measurement set. Please specify with parameter 'MS_INPUT_SCIENCE'."
	fi
    else
	echo "You must set either 'SB_SCIENCE' (scheduling block number) or 'MS_INPUT_SCIENCE' (Science observation measurement set)."
    fi
fi
if [ "$MS_INPUT_SCIENCE" == "" ]; then
    if [ $DO_SCIENCE_FIELD == true ]; then
	echo "Parameter 'MS_INPUT_SCIENCE' not defined. Turning off science processing with DO_SCIENCE_FIELD=false."
    fi
    DO_SCIENCE_FIELD=false
fi

####################
# Check the number of beams

nbeam=`echo $BEAM_MAX $BEAM_MIN | awk '{print $1-$2+1}'`

# Turn off mosaicking if there is just a single beam
if [ $nbeam -eq 1 ]; then
    if [ $DO_MOSAIC == true ]; then
	echo "Only have a single beam to process, so setting DO_MOSAIC=false"
    fi
    DO_MOSAIC=false
fi

####################
# Parameters required for continuum imaging
####

# nchanContSci = number of channels after averaging
nchanContSci=`echo $NUM_CHAN $NUM_CHAN_TO_AVERAGE | awk '{print $1/$2}'`

# nworkergroupsSci = number of worker groups, used for MFS imaging. 
nworkergroupsSci=`echo $NUM_TAYLOR_TERMS | awk '{print 2*$1-1}'`

# total number of CPUs required for MFS continuum imaging, including
# the master
NUM_CPUS_CONTIMG_SCI=`echo $nchanContSci $nworkergroupsSci | awk '{print $1*$2+1}'`


####################
# Define the beam arrangements for linmos
. ${SCRIPTDIR}/beamArrangements.sh

# Fix the direction string for linmos - don't need the J2000 bit
linmosFeedCentre=`echo $directionSci | awk -F',' '{printf "%s,%s]",$1,$2}'`
