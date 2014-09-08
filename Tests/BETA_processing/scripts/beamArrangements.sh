#!/usr/bin/env bash
#
# Defines the beam arrangements for predetermined named sets
#
# (c) Matthew Whiting, ATNF, 2014

if [ $doLinmos == true ]; then

    if [ $linmosBeams == "diamond" ]; then
	if [ $linmosBand == 1 ]; then
	    linmosBeams="# This is the arrangement for the diamond pattern in band 1
linmos.feeds.BEAM0      = [ 0.0,    0.0]
linmos.feeds.BEAM1      = [ 0.0,   -1.66]
linmos.feeds.BEAM2      = [ 0.0,    1.66]
linmos.feeds.BEAM3      = [ 1.44,  -0.83]
linmos.feeds.BEAM4      = [-1.44,   0.83]
linmos.feeds.BEAM5      = [ 1.44,   0.83]
linmos.feeds.BEAM6      = [-1.44,  -0.83]
linmos.feeds.BEAM7      = [ 2.875,  0.0]
linmos.feeds.BEAM8      = [ -2.875, 0.0]"
	elif [ $linmosBand == 2 ]; then
	    linmosBeams="# This is the arrangement for the diamond pattern in band 2
linmos.feeds.BEAM0      = [ 0.0,   0.0]
linmos.feeds.BEAM1      = [ 0.0,  -1.28]
linmos.feeds.BEAM2      = [ 0.0,   1.28]
linmos.feeds.BEAM3      = [ 1.11, -0.64]
linmos.feeds.BEAM4      = [-1.11,  0.64]
linmos.feeds.BEAM5      = [ 1.11,  0.64]
linmos.feeds.BEAM6      = [-1.11, -0.64]
linmos.feeds.BEAM7      = [ 2.22,  0.0]
linmos.feeds.BEAM8      = [-2.22,  0.0]"
	elif [ $linmosBand == 3 ]; then 
	    linmosBeams="# This is the arrangement for the diamond pattern in band 3
linmos.feeds.BEAM0      = [ 0.0,    0.0]
linmos.feeds.BEAM1      = [ 0.0,   -1.0]
linmos.feeds.BEAM2      = [ 0.0,    1.0]
linmos.feeds.BEAM3      = [ 0.866, -0.5]
linmos.feeds.BEAM4      = [-0.866,  0.5]
linmos.feeds.BEAM5      = [ 0.866,  0.5]
linmos.feeds.BEAM6      = [-0.866, -0.5]
linmos.feeds.BEAM7      = [ 1.732,  0.0]
linmos.feeds.BEAM8      = [-1.732,  0.0]"
	else
	    echo "Beam arrangement for ${linmosBeams} pattern not defined for band=${linmosBand}"
	    linmosBeams=""
	fi

    fi

    if [ "$linmosBeams" == "" ]; then
	echo "Cannot determine beam arrangement for linmos. Setting doLinmos=false."
	doLinmos=false
    fi

fi

