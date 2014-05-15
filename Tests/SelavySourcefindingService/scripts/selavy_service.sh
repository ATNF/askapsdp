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
#
# The files to be read from the FTP_INCOMING directory have this signature:
#
# New fits files: <MD5SUM.fits>
# New fits completation files <MD5SUM.fits.md5>
#
# New job files: <UUID.job>
# New job completation files <UUID.job.md5>
#
# Where MD5SUM is the actual MD5 has for the file, and UUID is a UUID
# generated with uuidgen(1) or similar

# Source configuration
. /exported/duchampsvc/scripts/config.sh

# Print a datestamp each time the script is run
echo "INFO: `date`"

############################################

# Check the size of the disk. If it is >= 99% full, then we stop the
# service and send to the manageer
DISKSIZE=`df -hP ${DUCHAMP_SVC_BASE} | tail -1 | sed s/%// | awk '{print $5}'`
if [ $DISKSIZE -ge 99 ]; then
    echo "ERROR: Disk full. Stopping service"

    ${SCRIPTS_DIR}/startStop.sh stop

    /usr/sbin/sendmail $FROM_EMAIL <<EOF
To: $MANAGER_EMAIL
From: $FROM_EMAIL
Subject: [Selavy] Selavy Service - DISK FULL!
The disk hosting the Selavy Source-finding Service is essentially full:

> df -h ${DUCHAMP_SVC_BASE}
`df -h ${DUCHAMP_SVC_BASE}`

The service has been stopped - do something to fix this!
EOF

    exit 1

fi

# Check for the existence of the images directory. If we can't see it,
# stop the service and notify the manager.
#if ! ( test -e ${IMAGE_DIR} ) ; then
if [ ! -e ${IMAGE_DIR} ]; then
    echo "ERROR: Image directory ${IMAGE_DIR} not accessible. Stopping service"

    ${SCRIPTS_DIR}/startStop.sh stop

    /usr/sbin/sendmail $FROM_EMAIL <<EOF
To: $MANAGER_EMAIL
From: $FROM_EMAIL
Subject: [Selavy] Selavy Service - IMAGE DIRECTORY INACCESSIBLE!
The image directory ${IMAGE_DIR} is not accessible.

The service has been stopped - do something to fix this!
EOF

    exit 1

fi

# Check for the existence of the compute nodes. If ping fails to any
# of them, stop the service and notify the manager.
nodes=(minicp00 minicp01 minicp02 minicp03 minicp04 minicp05 minicp06 minicp07)
numpingerrs=0
for node in ${nodes[@]}; do
    if [ $numpingerrs -eq 0 ]; then
	numpingerrs=`ping -c 1 -q ${node} | grep -c error`
	if [ $numpingerrs -gt 0 ]; then
	    badnode=$node
	fi
    fi
done
if [ $numpingerrs -gt 0 ]; then
 
    echo "ERROR: compute node ${badnode} not accessible. Stopping service"

    ${SCRIPTS_DIR}/startStop.sh stop

    /usr/sbin/sendmail $FROM_EMAIL <<EOF
To: $MANAGER_EMAIL
From: $FROM_EMAIL
Subject: [Selavy] Selavy Service - COMPUTE NODES OFFLINE!
At least one of the minicp compute nodes - ${badnode} - is not accessible.

The service has been stopped - do something to fix this!
EOF

    exit 1
fi 

############################################

# For each user account
for user in ${FTP_USERS}; do

    FTP_INCOMING_DIR=${FTP_BASE_DIR}/${user}/$FTP_INCOMING
#    echo "DEBUG: Checking incoming directory ${FTP_INCOMING_DIR}"

    #
    # Transfer new images/cubes from FTP_INCOMING_DIR
    #
    NEWFILES=`ls ${FTP_INCOMING_DIR}/*fits.md5 2> /dev/null`
#    if [ $? != 0 ]; then
#        echo "DEBUG: No new image files to transfer"
#    fi

    for MD5FILE in ${NEWFILES}; do
        # First check the checksum of the file is the same as its filename.
        # If not, ten the user will never be able to use it
        IMGFILE=`echo ${MD5FILE} | sed 's/.fits.md5//g'`.fits

        # Handle case where the *.fits.md5 file is present but not the
        # corresponding *.fits file
        if [ ! -f $IMGFILE ]; then
            echo "WARNING: Image file `basename ${IMGFILE}` not present"
            rm -f "${MD5FILE}"
            continue
        fi

        # Verify the md5 hash in the filename is the same as the actual
        # md5 hash of the fits file
        HASH_IN_FILENAME=`basename $IMGFILE | sed 's/.fits//g'`
        HASH_ACTUAL=`md5sum $IMGFILE | cut -f1 -d" "`
        if [ $HASH_IN_FILENAME != $HASH_ACTUAL ]; then
            echo "WARNING: Received file with md5sum differing from the filename"
            rm -f "${IMGFILE}"
            rm -f "${MD5FILE}"
            continue
        fi

        # Move from the FTP directory to the image repository
        DEST_FILE="${IMAGE_DIR}/`basename ${IMGFILE}`"
        if [ ! -f ${DEST_FILE} ]; then
            echo "INFO: Copying image file ${MD5FILE}"
            cp -r "${IMGFILE}" "${IMAGE_DIR}"
            ERR=$?
            if [ $ERR != 0 ]; then
                echo "ERROR: Error $ERR during file copy, leaving image in source location"
                continue
            fi
        else
            echo "INFO: File `basename ${IMGFILE}` already exists, will not re-copy"
        fi

        # Now images are confirmed moved, delete from source directory
        rm -f "${IMGFILE}"
        rm -f "${MD5FILE}"
    done

    #
    # Transfer and execute new job requests from FTP_INCOMING_DIR
    # 
    NEWFILES=`ls ${FTP_INCOMING_DIR}/*job.md5 2> /dev/null`
#    if [ $? != 0 ]; then
#        echo "DEBUG: No new job request files to transfer"
#    fi

    for MD5FILE in ${NEWFILES}; do
        JOB_FILE=`echo $MD5FILE | sed 's/.job.md5//g'`.job

        # Handle case where the *.job.md5 file is present but not the
        # corresponding *.job file
        if [ ! -f $JOB_FILE ]; then
            echo "WARNING: Job request file `basename ${JOB_FILE}` not present"
            rm -f "${MD5FILE}"
            continue
        fi

        # Move from the FTP directory to the job repository
        DEST_FILE="${JOB_DIR}/`basename ${JOB_FILE}`"
        if [ ! -f ${DEST_FILE} ]; then
            echo "INFO: Copying job request file ${JOB_FILE}"
            cp -r "${JOB_FILE}" "${JOB_DIR}"
            ERR=$?
            if [ $ERR != 0 ]; then
                echo "ERROR: Error $ERR during file copy, leaving job file in source location"
                continue
            fi
        else
            # This is a bit unusual, jobs should be created with a uuid
            echo "WARN: Job `basename ${JOB_FILE}` already exists, will not re-copy"
        fi

        # Now job request files are confirmed moved, delete from source directory
        rm -f "${JOB_FILE}"
        rm -f "${MD5FILE}"

        # Finally, run the job
        ${SCRIPTS_DIR}/runjob.sh ${JOB_DIR}/`basename ${JOB_FILE}` $user
    done
done
