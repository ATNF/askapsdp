#!/bin/bash -l
#
# Copyright (c) 2010 CSIRO
# Australia Telescope National Facility (ATNF)
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# PO Box 76, Epping NSW 1710, Australia
# atnf-enquiries@csiro.au
#
# This file is part of the ASKAP software distribution.
#
# The ASKAP software distribution is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# Description:
# ------------
# Simple way to start and stop the source finding service. All it does
# is change the crontab, and write the correct status to a file that
# can be displayed on the webpage.

function writeHTML {
FILE=$1
STATUS=$2

if [ $STATUS == "start" ]; then

    STATUSTEXT="<SPAN class=\"run\">&nbsp;&nbsp;RUNNING&nbsp;&nbsp;</SPAN>"

else 

    STATUSTEXT="<SPAN class=\"stop\">&nbsp;&nbsp;HALTED&nbsp;&nbsp;</SPAN>"

fi

cat > $FILE <<EOF
<div id="status">
<p>
Current status of the source finding service: $STATUSTEXT
</p>
</div>
EOF

}

# Source configuration
. /exported/duchampsvc/scripts/config.sh

# Check usage
if [ $# -ne 1 ]; then
    echo "usage: startStop.sh start|stop"
    exit 1
fi

STATUSFILE=${WEBDIR}/ServiceStatus.html


if [ $1 == "start" ]; then

    #Change crontab
    crontab -l | awk '{if (($7 == "/exported/duchampsvc/scripts/selavy_service.sh" || $7=="/exported/duchampsvc/scripts/htmlgen.sh" || $7=="/exported/duchampsvc/scripts/cleanup.sh" || $7=="/exported/duchampsvc/scripts/stats.sh")&&($1=="#")) print $2,$3,$4,$5,$6,$7,$8,$9,$10; else print $0}' | crontab - 
    
    #Change status webpage
    writeHTML $STATUSFILE start

elif [ $1 == "stop" ]; then

    #Change crontab
    crontab -l | awk '{if (($6 == "/exported/duchampsvc/scripts/selavy_service.sh") || ($6=="/exported/duchampsvc/scripts/htmlgen.sh") || ($6=="/exported/duchampsvc/scripts/cleanup.sh" || $6=="/exported/duchampsvc/scripts/stats.sh")) printf "# %s\n",$0; else print $0}' | crontab - 

    #Change status webpage
    writeHTML $STATUSFILE stop

else

    echo "usage: startStop.sh start|stop"
    exit 1

fi
    
