#!/usr/bin/env bash
#
# Defines the beam arrangements for predetermined named sets. This makes use
# of the ACES footprint.py tool to extract the beam offsets for a named beam
# footprint. The following variables must be defined: $BEAM_FOOTPRINT_NAME,
# $FREQ_BAND_NUMBER, $BEAM_FOOTPRINT_PA.
# Upon return, either $LINMOS_BEAM_OFFSETS will be set, or $DO_MOSAIC will have been
# set to false.
#
# (c) Matthew Whiting, CSIRO ATNF, 2014

if [ $DO_MOSAIC == true ]; then

    if [ "$LINMOS_BEAM_OFFSETS" == "" ]; then
	# Beam pattern for linmos not defined. We need to use footprint.py to work it out.

	if [ "`which footprint.py`" == "" ]; then
	    # If we are here, footprint.py is not in our path. Give an error message and turn off linmos
	    
	    echo "Cannot find 'footprint.py', so cannot determine beam arrangement for linmos. Setting DO_MOSAIC=false."
	    DO_MOSAIC=false

	else

	    if [ "$BEAM_FOOTPRINT_NAME" == "diamond" ] || [ "$BEAM_FOOTPRINT_NAME" == "rhombus" ] || [ "$BEAM_FOOTPRINT_NAME" == "line" ] \
		|| [ "$BEAM_FOOTPRINT_NAME" == "trapezoid2" ] || [ "$BEAM_FOOTPRINT_NAME" == "trapezoid3" ]\
		|| [ "$BEAM_FOOTPRINT_NAME" == "octagon" ] || [ "$BEAM_FOOTPRINT_NAME" == "3x3" ] || [ "$BEAM_FOOTPRINT_NAME" == "square" ]; then

	    # If we have an allowed footprint name, then run footprint.py and extract the beam offsets

		footprintOut="$parsets/footprintOutput.txt"
		echo "footprint.py -t -n $BEAM_FOOTPRINT_NAME -b $FREQ_BAND_NUMBER -a $BEAM_FOOTPRINT_PA > ${footprintOut}"
		footprint.py -t -n $BEAM_FOOTPRINT_NAME -b $FREQ_BAND_NUMBER -a $BEAM_FOOTPRINT_PA > ${footprintOut}

		if [ `wc -l $footprintOut | awk '{print $1}'` != 0 ]; then
		    
		# The offsets are taken from the appropriate column, with the RA offset flipped in sign.

		    beamsOut="$parsets/beamOffsets.txt"
		    grep -A9 Beam ${footprintOut} | tail -n 9 | sed -e 's/(//g' | sed -e 's/)//g' | awk '{printf "linmos.feeds.beam%d = [%6.3f, %6.3f]\n",$1,-$4,$5}' > ${beamsOut}
		    LINMOS_BEAM_OFFSETS=`cat ${beamsOut}`

		else
		  
		    # Something went wrong.
		    echo "The command 'footprint.py  -n $BEAM_FOOTPRINT_NAME -b $FREQ_BAND_NUMBER -a $BEAM_FOOTPRINT_PA' failed."
		
		fi

	    else

		# The footprint name is not one of the allowed values.
		echo "Beam arrangement for ${BEAM_FOOTPRINT_NAME} footprint not defined. Setting DO_MOSAIC=false."
		DO_MOSAIC=false

	    fi

	
	    if [ "$LINMOS_BEAM_OFFSETS" == "" ]; then
		# If we get to here, we have not set the offsets for the beams, so turn off the linmos mode
		DO_MOSAIC=false
	    fi
	
	fi	
	
    fi

fi

