#!/usr/bin/env bash
#
# Process the science field observations: split out per beam, flag,
# apply the bandpass solution, average spectrally, image the continuum
# with or without self-calibration, and image the spectral-line
# data. Finally, mosaic the continuum images.
#
# (c) Matthew Whiting, CSIRO ATNF, 2014

echo "Setting up and calibrating the science observation"

FLAG_IMAGING_DEP=""

for((BEAM=${BEAM_MIN}; BEAM<=${BEAM_MAX}; BEAM++)); do
    
    sedstr="s/\.ms/_${BEAM}\.ms/g"
    msSci=`echo $msSciBase | sed -e $sedstr`

    . ${SCRIPTDIR}/splitFlagScience.sh
    
    . ${SCRIPTDIR}/applyBandpassScience.sh

    . ${SCRIPTDIR}/averageScience.sh

    if [ $doSelfcal == true ]; then
	. ${SCRIPTDIR}/continuumImageScienceSelfcal.sh
    else
	. ${SCRIPTDIR}/continuumImageScience.sh
    fi

    . ${SCRIPTDIR}/spectralImageScience.sh

done

. ${SCRIPTDIR}/linmos.sh
