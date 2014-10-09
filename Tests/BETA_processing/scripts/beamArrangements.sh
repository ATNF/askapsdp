#!/usr/bin/env bash
#
# Defines the beam arrangements for predetermined named sets. This makes use
# of the ACES footprint.py tool to extract the beam offsets for a named beam
# footprint. The following variables must be defined: $beamFootprintName,
# $linmosBand, $beamFootprintPA.
# Upon return, either $linmosBeams will be set, or $doLinmos will have been
# set to false.
#
# (c) Matthew Whiting, CSIRO ATNF, 2014

if [ $doLinmos == true ]; then

    if [ "$linmosBeams" == "" ]; then
	# Beam pattern for linmos not defined. We need to use footprint.py to work it out.

	if [ "`which footprint.py`" == "" ]; then
	    # If we are here, footprint.py is not in our path. Give an error message and turn off linmos
	    
	    echo "Cannot find 'footprint.py', so cannot determine beam arrangement for linmos. Setting doLinmos=false."
	    doLinmos=false

	else

	    if [ "$beamFootprintName" == "diamond" ] || [ "$beamFootprintName" == "rhombus" ] || [ "$beamFootprintName" == "line" ] \
		|| [ "$beamFootprintName" == "trapezoid2" ] || [ "$beamFootprintName" == "trapezoid3" ]\
		|| [ "$beamFootprintName" == "octagon" ] || [ "$beamFootprintName" == "3x3" ] || [ "$beamFootprintName" == "square" ]; then

	    # If we have an allowed footprint name, then run footprint.py and extract the beam offsets

		footprintOut="$parsets/footprintOutput.txt"
		echo "footprint.py -t -n $beamFootprintName -b $linmosBand -a $beamFootprintPA > ${footprintOut}"
		footprint.py -t -n $beamFootprintName -b $linmosBand -a $beamFootprintPA > ${footprintOut}

		if [ `wc -l $footprintOut | awk '{print $1}'` != 0 ]; then
		    
		# The offsets are taken from the appropriate column, with the RA offset flipped in sign.

		    beamsOut="$parsets/beamOffsets.txt"
		    grep -A9 Beam ${footprintOut} | tail -n 9 | sed -e 's/(//g' | sed -e 's/)//g' | awk '{printf "linmos.feeds.beam%d = [%6.3f, %6.3f]\n",$1,-$4,$5}' > ${beamsOut}
		    linmosBeams=`cat ${beamsOut}`

		else
		  
		    # Something went wrong.
		    echo "The command 'footprint.py  -n $beamFootprintName -b $linmosBand -a $beamFootprintPA' failed."
		
		fi

	    else

		# The footprint name is not one of the allowed values.
		echo "Beam arrangement for ${beamFootprintName} footprint not defined. Setting doLinmos=false."
		doLinmos=false

	    fi

	
	    if [ "$linmosBeams" == "" ]; then
		# If we get to here, we have not set the offsets for the beams, so turn off the linmos mode
		doLinmos=false
	    fi
	
	fi	
	
    fi

fi

