#!/bin/bash -l 

if [ $doCorrupt == true ]; then
    if [ $doCal == true ]; then
	tag="CC"
	mstag="CALIBRATED"
	imtag="cal"
    else
	tag="CU"
	mstag="CORRUPTED"
	imtag="corrupt"
    fi
else
    tag="UU"
    mstag="UNCOR"
    imtag="uncor"
fi
