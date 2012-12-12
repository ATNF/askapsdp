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
# Removes files/directories per retention period, which may be either 
# based on creation time or access time
#

function cleanup {
CLEANUP_DIR=$1
RETENTION_PERIOD=$2

FILES=`ls ${CLEANUP_DIR} 2> /dev/null`
for file in ${FILES}; do
    # Get the owners permissions from stat
    PERM=`stat --format="%a" ${CLEANUP_DIR}/${file} | cut -c1`
    TYPE=`stat --format="%F" ${CLEANUP_DIR}/${file}`

    # Only procees to check/delete if the file perms are read/write and it's not a symbolic link
    if [ ${PERM} != "4" ] && [ "${TYPE}" != "symbolic link" ]; then
        if [ "$3" == "ctime" ]; then
            FILE_TIME=`stat --format=%Z ${CLEANUP_DIR}/${file}`
        else 
            FILE_TIME=`stat --format=%X ${CLEANUP_DIR}/${file}`
        fi
        DELETE_TIME=`expr ${FILE_TIME} + ${RETENTION_PERIOD}`
        CURRENT_TIME=`date +%s`
        if [ $CURRENT_TIME -gt $DELETE_TIME ]; then
            echo "INFO: Removing ${CLEANUP_DIR}/${file}"
            rm -rf "${CLEANUP_DIR}/${file}"
        fi
    fi
done
}

# Source configuration
source /exported/duchampsvc/scripts/config.sh

# Print a timestamp each time script is run
echo "INFO: `date`"

# Retention period in seconds
RETENTION_PERIOD=1209600
#RETENTION_PERIOD=60480000

# First arg is the directory to check, second arg the retention time in seconds,
# third arg indicates if the retention is based on access time (atime) or creation
# time (ctime)

# Delete the working directories after some time. These are only needed
cleanup ${JOB_DIR} ${RETENTION_PERIOD} ctime
cleanup ${WORK_DIR_BASE} ${RETENTION_PERIOD} ctime

# Delete images which have not been accessed for a long time
# If the image file is chmod to have no write permissions, it will
# not be subject to the retention period.
cleanup ${IMAGE_DIR} ${RETENTION_PERIOD} atime

# It is the selavysvc script responsibility to delete job and image
# files from the FTP incoming directories, but it will only delete them once the
# upload/submission is complete (indicated by the existance of the md5 file)
# and only once the data has been copied over the the working directories
# The below will delete those files which have not yet been completed, and
# is really aimed to catch aborted uploads.
#
# The outgoing directory is also cleaned up periodically, so users must
# download their results in a timely manner or risk loosing them.
for user in ${FTP_USERS}; do
    DIR=${FTP_BASE_DIR}/${user}/${FTP_INCOMING}
    cleanup ${DIR} ${RETENTION_PERIOD} ctime
    DIR=${FTP_BASE_DIR}/${user}/${FTP_OUTGOING}
    cleanup ${DIR} ${RETENTION_PERIOD} ctime
done
