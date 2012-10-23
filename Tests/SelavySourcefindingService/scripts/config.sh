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

# Base diretory
DUCHAMP_SVC_BASE=/exported/duchampsvc

FTP_USERS="duchamp000 duchamp001 duchamp002 duchamp003 duchamp004 duchamp005 duchamp006 duchamp007 duchamp008 duchamp009 duchamp010"

FTP_BASE_DIR=/nfs/ftp-vusers

# Directory where this script will look for new fits files and job
# request templates
FTP_INCOMING=incoming

# Directory for log file and source catalog to be copied to
FTP_OUTGOING=outgoing

# Directories where the job can find the images
#IMAGE_DIR=${DUCHAMP_SVC_BASE}/images
#IMAGE_DIR=/DATA/GIANT_0/work0/duchampsvc/images
IMAGE_DIR=/DATA/DELPHINUS_1/duchampsvc/images

# Directory where the *.qsub files should be placed
JOB_DIR=${DUCHAMP_SVC_BASE}/jobs

# Working directory base for the batch job
WORK_DIR_BASE=${DUCHAMP_SVC_BASE}/workdir

SCRIPTS_DIR=${DUCHAMP_SVC_BASE}/scripts

LOGS_DIR=${DUCHAMP_SVC_BASE}/logs

# Email address from which the completion emails will come from
FROM_EMAIL=matthew.whiting@csiro.au

# Email address of the person managing the service (may be the same as above, but need not)
MANAGER_EMAIL=matthew.whiting@csiro.au

# Number of CPU cores available per node.
CORES_PER_NODE=4

# Limit the size of the job which can be submitted (note: minicp has 32 cores)
MAX_CORES=16

# Limit the size of images (mask, spectra) to be included in the tarball (in K)
#MAX_IMG_SIZE=102400
MAX_IMG_SIZE=51200

# ASKAP Software location
# The various scripts use this instead of the more typical ASKAP_ROOT
#ASKAP_SOFTWARE=${DUCHAMP_SVC_BASE}/askapsoft
#ASKAP_SOFTWARE=${DUCHAMP_SVC_BASE}/askapsoft-SFS-1.1
#ASKAP_SOFTWARE=${DUCHAMP_SVC_BASE}/askapsoft-SFS-1.2
#ASKAP_SOFTWARE=${DUCHAMP_SVC_BASE}/askapsoft-SFS-1.3
#ASKAP_SOFTWARE=${DUCHAMP_SVC_BASE}/askapsoft-SFS-1.4
#ASKAP_SOFTWARE=${DUCHAMP_SVC_BASE}/askapsoft-SFS-1.5
#ASKAP_SOFTWARE=${DUCHAMP_SVC_BASE}/askapsoft-SFS-1.6
#ASKAP_SOFTWARE=${DUCHAMP_SVC_BASE}/askapsoft-SFS-1.7
#
ASKAP_SOFTWARE=/exported/whi550/ASKAP/askapsoft

# Where the web page showing the status should go
WEBDIR=/u/whi550/www

