#!/bin/bash
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
# For each work directory, extract info such as jobID, email, status,
# number of cores used, time used, ...


# Source configuration
. /exported/duchampsvc/scripts/config.sh

THISLOG=${LOGS_DIR}/stats-`date +%F-%H:%M`.log

for JOBID in `\ls -t ${WORK_DIR_BASE}`; do
    STATUS_FILE="${WORK_DIR_BASE}/${JOBID}/status.txt"
    if [ -f ${STATUS_FILE} ]; then
	STATUS=`cat ${STATUS_FILE}`
    fi
    SUBMIT="${WORK_DIR_BASE}/${JOBID}/submit.qsub"
    if [ -f ${SUBMIT} ]; then
	SST_ID=`grep duchamp ${SUBMIT} | grep PBS | grep -v duchampsvc | awk '{print $3}'`
	NCPUS=`grep PBS ${SUBMIT} | grep nodes | sed -e 's/=/ /g' | sed -e 's/:/ /g' | awk '{print $4*$6}'`
    else
	SST_ID="---"
	NCPUS=0
    fi

    PARSET="${WORK_DIR_BASE}/${JOBID}/selavy-${JOBID}.in"
    EMAIL=`grep email ${PARSET} | awk '{print $3}'`

    if [ -f ${JOB_DIR}/${JOBID}.job ]; then
	DATE=`stat --format="%z" ${JOB_DIR}/${JOBID}.job | cut -c1-19 | sed -e 's/ /-/g'`
    else
	DATE=`stat --format="%z" ${SUBMIT} | cut -c1-19 | sed -e 's/ /-/g'`
    fi

    echo $JOBID $DATE $EMAIL $SST_ID $NCPUS \"$STATUS\" >> ${THISLOG}

done

BIGSTATSFILE=${LOGS_DIR}/statsSummary.csv
cat > ${BIGSTATSFILE} <<EOF
# JOBID EMAIL SST NCPUS STATUS
EOF
tempfile=${LOGS_DIR}/tempstats.log
rm -rf ${tempfile}
for STATS in `\ls ${LOGS_DIR}/stats-*`; do
    grep -v Running ${STATS} | grep -v Queued | sed -e 's/Completed - Failed/Completed-Failed/g' | sed -e 's/Rejected - Exceeds available # of cores/Rejected_-_Exceeds_available_#_of_cores/g' | awk 'NF==6' | sed -e 's/ /,/g' >> ${tempfile}
done
sort -k1 ${tempfile} | uniq -w 37 >> ${BIGSTATSFILE}
