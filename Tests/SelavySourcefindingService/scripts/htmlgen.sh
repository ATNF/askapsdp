#!/usr/bin/env bash
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
#

# Source configuration
. /exported/duchampsvc/scripts/config.sh

OUTPUT=${WEBDIR}/SelavyStatus.html

cat > ${OUTPUT} <<EOF
<table border="1">
<tr>
<th>JobID</th>
<th>Submission time</th>
<th>Status</th>
</tr>

EOF

#for JOBID in `ls -t ${WORK_DIR_BASE}`; do
for STATUS_FILE in `ls -t ${WORK_DIR_BASE}/*/status.txt`; do
#    STATUS_FILE="${WORK_DIR_BASE}/${JOBID}/status.txt"
    JOBID=`echo $STATUS_FILE | sed -e 's|/| |g' | awk '{print $(NF-1)}'`
    if [ -f ${STATUS_FILE} ]; then
	if [ -e ${WORK_DIR_BASE}/${JOBID}/submit.qsub ]; then
	    SUBMITTIME=`stat --format="%z" ${WORK_DIR_BASE}/${JOBID}/submit.qsub | cut -c 1-19`
	else
	    SUBMITTIME=`stat --format="%z" ${STATUS_FILE} | cut -c 1-19`
	fi
        STATUS=`cat ${STATUS_FILE}`
#        echo "${JOBID} - ${STATUS}"
        echo "<tr>" >> ${OUTPUT}
        echo "<td>${JOBID}</td>" >> ${OUTPUT}
	echo "<td>${SUBMITTIME}</td>" >> ${OUTPUT}
        echo "<td>${STATUS}</td>" >> ${OUTPUT}
        echo "</tr>" >> ${OUTPUT}
    fi
done

cat >> ${OUTPUT} <<EOF
</table>

<p>Last update: `date +"%F %r"`</p>

EOF
