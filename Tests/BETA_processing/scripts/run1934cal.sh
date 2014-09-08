#!/usr/bin/env bash

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


