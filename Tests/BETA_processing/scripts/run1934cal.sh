#!/usr/bin/env bash
#
# Process the 1934-638 calibration observations: split out per beam,
# flag, then find the bandpass solution
#
# (c) Matthew Whiting, CSIRO ATNF, 2014

mslist=""
FLAG_1934_DEP=""

if [ $doFlag1934 == true ]; then

    for((BEAM=${BEAM_MIN}; BEAM<=${BEAM_MAX}; BEAM++)); do
	
	. ${SCRIPTDIR}/splitFlag1934.sh

    done

fi

if [ $doFindBandpass == true ]; then

    . ${SCRIPTDIR}/findBandpassCal.sh

fi


