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
#

# Source configuration
. /exported/duchampsvc/scripts/config.sh

# This needs to be exported for qsub. The value is set in config.sh above
export ASKAP_SOFTWARE

# Check usage
if [ $# -ne 2 ]; then
    echo "usage: runjob.sh <jobfile> <user>"
    exit 1
fi

# Print a log message
JOB_FILE=$1
USER_NAME=$2
echo "INFO: Processing job file ${JOB_FILE} for ${USER_NAME}"

# Outgoing FTP directory is per user account
FTP_OUTGOING_DIR=${FTP_BASE_DIR}/${USER_NAME}/${FTP_OUTGOING}

# Obtain the UUID so we can uniquely identify and track the job
UUID=`basename "${JOB_FILE}" | cut -f1 -d.`

# Create the workdir
WORK_DIR=${WORK_DIR_BASE}/${UUID}
mkdir ${WORK_DIR}
cd ${WORK_DIR}

cp ${JOB_FILE} input_`basename ${JOB_FILE}`

# Get the email address for completion reports to be sent to
EMAIL_LINE=`grep "Selavy.email" $JOB_FILE | grep -e '^\#' -v`
if [ $? -ne 0 ]; then
    echo "WARNING: Email address not present in job file. Aborting job"
    echo "Rejected - No email address" > ${WORK_DIR}/status.txt
    exit 1
fi
REQUESTER_EMAIL=`echo $EMAIL_LINE | cut -f 2 -d"=" | sed -e 's/^[ \t]*//'`
echo "INFO: User ${USER_NAME} with email ${REQUESTER_EMAIL} has submitted job with ID ${UUID}"

#
# Create the selavy parset file
#

# This is the input parset unique to this job, storing only parameters that are to be used & ignoring commented-out ones.
PARSETBASE=selavy-${UUID}.in
PARSET=${WORK_DIR}/${PARSETBASE}


# Copy the user params over, except the image name. Ignore commented-out params.
grep Selavy $JOB_FILE | grep -e '^\#' -v | grep -v "Selavy.image" | grep -v "Selavy.weightsimage" > ${PARSET}

# Add the image separatly, so the images directory prefix can be added
IMG_FILE=`grep "Selavy.image" ${JOB_FILE} | grep -e '^\#' -v | cut -f 2 -d"=" | sed -e 's/^[ \t]*//'`
echo "Selavy.image = ${IMAGE_DIR}/${IMG_FILE}" >> ${PARSET}

# Add the weights image separately also, if it is requested
WEIGHTS_IMG=`grep "Selavy.weightsimage" ${JOB_FILE} | grep -e '^\#' -v |  cut -f 2 -d"=" | sed -e 's/^[ \t]*//'`
if [ "$WEIGHTS_IMG" != "" ]; then
    echo "Selavy.weightsimage = ${IMAGE_DIR}/${WEIGHTS_IMG}" >> ${PARSET}
fi

CUBE_FILE=`grep "Selavy.extractSpectra.spectralCube" ${JOB_FILE} | grep -e '^\#' -v | cut -f 2 -d"=" | sed -e 's/^[ \t]*//'`
if [ "${CUBE_FILE}" != "" ]; then
    echo "Selavy.extractSpectra.spectralCube = ${IMAGE_DIR}/${CUBE_FILE}" >> ${PARSET}
fi
CUBE_FILE=`grep "Selavy.extractNoiseSpectra.spectralCube" ${JOB_FILE} | grep -e '^\#' -v | cut -f 2 -d"=" | sed -e 's/^[ \t]*//'`
if [ "${CUBE_FILE}" != "" ]; then
    echo "Selavy.extractNoiseSpectra.spectralCube = ${IMAGE_DIR}/${CUBE_FILE}" >> ${PARSET}
fi

# Changing the mask image, to make it easy to include in the tarball
MASK_IMG=`grep "Selavy.fileOutputMask" ${JOB_FILE} | grep -e '^\#' -v | cut -f 2 -d"=" | sed -e 's/^[ \t]*//'`
if [ "$MASK_IMG" != "" ]; then
    echo "Selavy.fileOutputMask = selavy-MASK-${MASK_IMG}" >> ${PARSET}
else
    FLAG_MASK=`grep "Selavy.flagOutputMask" ${JOB_FILE} | grep -e '^\#' -v | cut -f 2 -d"=" | sed -e 's/^[ \t]*//'`
    if [ ${FLAG_MASK} == "true" ] || [ ${FLAG_MASK} == "1" ]; then
	echo "Selavy.fileOutputMask = selavy-MASK-${IMG_FILE}" >> ${PARSET}
    fi
fi

#Changing the prefix of the output spectra, to make it easy to include in the tarball
FLAG_OUTPUT_SPECTRA=`grep "Selavy.extractSpectra" ${JOB_FILE} | grep -e '^\#' -v | cut -f 2 -d"=" | sed -e 's/^[ \t]*//'`
if [ ${FLAG_OUTPUT_SPECTRA} == "true" ] || [ ${FLAG_OUTPUT_SPECTRA} == "1" ]; then
    SPECTRA_BASE=`grep "Selavy.extractSpectra.spectralOutputBase" ${JOB_FILE} | grep -e '^\#' -v | cut -f 2 -d"=" | sed -e 's/^[ \t]*//'`
    echo "Selavy.extractSpectra.spectralOutputBase = selavy-SPECTRA-${SPECTRA_BASE}" >> ${PARSET}
fi

#Changing the prefix of the output noise spectra, to make it easy to include in the tarball
FLAG_OUTPUT_NOISESPECTRA=`grep "Selavy.extractNoiseSpectra" ${JOB_FILE} | grep -e '^\#' -v | cut -f 2 -d"=" | sed -e 's/^[ \t]*//'`
if [ ${FLAG_OUTPUT_NOISESPECTRA} == "true" ] || [ ${FLAG_OUTPUT_NOISESPECTRA} == "1" ]; then
    SPECTRA_BASE=`grep "Selavy.extractNoiseSpectra.spectralOutputBase" ${JOB_FILE} | grep -e '^\#' -v | cut -f 2 -d"=" | sed -e 's/^[ \t]*//'`
    echo "Selavy.extractNoiseSpectra.spectralOutputBase = selavy-NOISE-SPECTRA-${SPECTRA_BASE}" >> ${PARSET}
fi

#
# Determine how many nodes and processors per node need to be allocated
#
NSUB_X=`grep "Selavy.nsubx" ${JOB_FILE} | grep -e '^\#' -v | cut -f 2 -d"=" | sed -e 's/^[ \t]*//'`
if [ "$NSUB_X" == "" ]; then
    NSUB_X=1
fi
NSUB_Y=`grep "Selavy.nsuby" ${JOB_FILE} | grep -e '^\#' -v | cut -f 2 -d"=" | sed -e 's/^[ \t]*//'`
if [ "$NSUB_Y" == "" ]; then
    NSUB_Y=1
fi
NSUB_Z=`grep "Selavy.nsubz" ${JOB_FILE} | grep -e '^\#' -v | cut -f 2 -d"=" | sed -e 's/^[ \t]*//'`
if [ "$NSUB_Z" == "" ]; then
    NSUB_Z=1
fi
TOTAL_CORES=`expr $NSUB_X \* $NSUB_Y \* $NSUB_Z`

# Add one for the master process for multi-process job
if [ $TOTAL_CORES -gt 1 ]; then
    TOTAL_CORES=`expr $TOTAL_CORES + 1`
fi

if [ $TOTAL_CORES -gt $MAX_CORES ]; then
    echo "WARNING: Job exceeds MAX_CORES, will not submit"

    # Send an email to the user
    echo "To: $REQUESTER_EMAIL" > ${UUID}.mail
    echo "From: $FROM_EMAIL" >> ${UUID}.mail
    echo "Subject: Duchamp job sumission failed (FAILURE)" >> ${UUID}.mail
    echo >> ${UUID}.mail
    echo "JobId: ${UUID}" >> ${UUID}.mail
    echo >> ${UUID}.mail
    echo "Job submission failed. Job requires ${TOTAL_CORES} cores which exceeds limit of ${MAX_CORES}" >> ${UUID}.mail
#    /usr/sbin/sendmail $REQUESTER_EMAIL < ${UUID}.mail
    /usr/sbin/sendmail -t < ${UUID}.mail

    echo "Rejected - Exceeds available # of cores" > ${WORK_DIR}/status.txt
    exit 1
fi


# Determine number of nodes, and number of cores per node
#NNODES=`expr $TOTAL_CORES / $CORES_PER_NODE`
NNODES=`echo $TOTAL_CORES $CORES_PER_NODE | awk '{printf "%d", ($1-1)/$2 + 1}'`
MEM=`echo $NNODES | awk '{printf "%d",$1 * 12}'`
#if [ $NNODES -eq 0 ]; then
if [ $NNODES -eq 1 ]; then
    # A job of less than $CORES_PER_NODE gets only the number of cores it needs
#    NNODES=1
    PPN=`expr $TOTAL_CORES % $CORES_PER_NODE`
else
    # If more than one node is required, just allocate all cores for all nodes
    PPN=4
fi

###############################################################################
# NOTE - For now, to avoid running out of memory, just allocate the entire
# node. No sharing of nodes between jobs.
###############################################################################
PPN=4

#
# Build the qsub file
#
cat > ${WORK_DIR}/submit.qsub <<EOF
#!/bin/bash
#PBS -l nodes=${NNODES}:ppn=${PPN}
#PBS -l mem=${MEM}GB
#PBS -l walltime=24:00:00
#PBS -N ${USER_NAME}
#PBS -m n
#PBS -j oe
#PBS -v ASKAP_SOFTWARE

cd \$PBS_O_WORKDIR

source /usr/local/Modules/3.2.5/init/bash
module load openmpi

# Change status to running
echo "Running" > ${WORK_DIR}/status.txt

# copy log config file to working directory
cp ${SCRIPTS_DIR}/askap.log_cfg .

echo "Running duchamp service request with UUID ${UUID}"
#export AIPSPATH=${ASKAP_SOFTWARE}/Code/Components/Synthesis/testdata/current
export AIPSPATH=${ASKAP_SOFTWARE}/Code/Base/accessors/current

# Run duchamp
mpirun -bynode -np $TOTAL_CORES ${ASKAP_SOFTWARE}/Code/Components/Analysis/analysis/current/apps/selavy.sh -inputs ${PARSET} > ${UUID}.log
ERR=\$?

# Compress the log as it can be quite large
gzip ${UUID}.log

# Copy results to the outgoing FTP directory
if [ \$ERR -eq 0 ]; then
    for file in \`\ls selavy-*\`; do
        if [ \`du -sk \$file | cut -f 1\` -gt ${MAX_IMG_SIZE} ]; then
            mv \$file "TOO_BIG_\$file"
        fi
    done
    tar zcvf ${FTP_OUTGOING_DIR}/${UUID}.tgz selavy-* ${PARSETBASE}
    chmod 644 ${FTP_OUTGOING_DIR}/${UUID}.tgz
fi

# Copy the log file to the outgoing FTP directory
cp "${UUID}.log.gz" "${FTP_OUTGOING_DIR}"
chmod 644 "${FTP_OUTGOING_DIR}/${UUID}.log.gz"

if [ \$ERR -eq 0 ]; then
    # Send an email to the user
    echo "To: $REQUESTER_EMAIL" > ${UUID}.mail
    echo "From: $FROM_EMAIL" >> ${UUID}.mail
    echo "Subject: Duchamp job completed (SUCCESS)" >> ${UUID}.mail
    echo >> ${UUID}.mail
    echo "JobId: ${UUID}" >> ${UUID}.mail
    echo >> ${UUID}.mail
    echo "Duchamp job completed successfully. Results can be downloaded from the following location:" >> ${UUID}.mail
    echo "ftp://${USER_NAME}@ftp.atnf.csiro.au/outgoing/${UUID}.tgz" >> ${UUID}.mail
    echo >> ${UUID}.mail
    echo "A log of the job execution is available here:" >> ${UUID}.mail
    echo "ftp://${USER_NAME}@ftp.atnf.csiro.au/outgoing/${UUID}.log.gz" >> ${UUID}.mail

    echo >> ${UUID}.mail
    echo "============================ Original Job Request Template ============================" >> ${UUID}.mail
    cat $JOB_FILE >> ${UUID}.mail

#    /usr/sbin/sendmail $REQUESTER_EMAIL < ${UUID}.mail
    /usr/sbin/sendmail -t < ${UUID}.mail
fi

# Change status to completed
echo "Completed" > ${WORK_DIR}/status.txt

exit \$ERR
EOF

#
# Submit the job to the batch queue
# The job is submitted with a user hold. This is so the "watcher" job can be submitted
# without fear of this job completing before the watcher job can reference it as a 
# dependency.
#
JOBID=`qsub -h ${WORK_DIR}/submit.qsub 2>&1`
echo "Queued" > ${WORK_DIR}/status.txt
if [ $? == 0 ]; then
    echo "INFO: Submitted batch job ${JOBID} for request ${UUID}"
else
    echo "ERROR: Failed to submit batch job. Output: ${JOBID}"

    # Send an email to the user
    echo "To: $REQUESTER_EMAIL" > ${UUID}.mail
    echo "From: $FROM_EMAIL" >> ${UUID}.mail
    echo "Subject: Duchamp job completed (FAILURE)" >> ${UUID}.mail
    echo >> ${UUID}.mail
    echo "JobId: ${UUID}" >> ${UUID}.mail
    echo >> ${UUID}.mail
    echo "Job submission failed:" >> ${UUID}.mail
    echo "${JOBID}" >> ${UUID}.mail
#    /usr/sbin/sendmail $REQUESTER_EMAIL < ${UUID}.mail
    /usr/sbin/sendmail -t < ${UUID}.mail
fi


#                        ERROR HANDLING
# The above job may run out of time, or otherwise die without getting
# a chance to email the user. So submit a job to run afternotok. This
# job is then responsible for notifying the user in case of error. It
# deals with the case where selavy returns a non-zero exit code, as
# well as where the entire job is aborted (say due to time limit being
# exceeded)

#
# Build the qsub file
#
cat > ${WORK_DIR}/afternotok.qsub <<EOF
#!/bin/bash
#PBS -l nodes=1:ppn=1
#PBS -l walltime=00:10:00
#PBS -N ${USER_NAME}-w
#PBS -m n
#PBS -j oe

cd \$PBS_O_WORKDIR

# Change status to completed failed
echo "Completed - Failed" > ${WORK_DIR}/status.txt

# This sleep avoids a race condition between the execution of this job and the
# copying of the stdout/stderr files from the compute node to the PBS_O_WORKDIR
# The stdout file is used below and if it has not yet been copied, checking for
# certain errors doesn't occur.
sleep 30

# The below was an attempt to workaround the problem on minicp where upon login
# one is presented with the following question:
# > Package available: AIPS++ ...Astronomical Information Processing System
# > Do you want to use it? (y/n) [n]: Recording -AIPS++ in .login.packages.
# The plan below was to simply resubmit the job if this question is posed.
#LOGINPACKAGES=\`grep -c login.packages ${USER_NAME}.o*\`
#if [ \${LOGINPACKAGES} -ne 0 ]; then
#    NEWJOBID=\`qsub -h ${WORK_DIR}/submit.qsub 2>&1\`
#    NOTOKJOBID=\`qsub -W depend=afternotok:\${NEWJOBID} ${WORK_DIR}/afternotok.qsub 2>&1\`
#    qrls \$NEWJOBID
#    exit 0
#fi

# Compress the log as it can be quite large
gzip ${UUID}.log

# Copy the log file to the outgoing FTP directory
cp "${UUID}.log.gz" "${FTP_OUTGOING_DIR}"
chmod 644 "${FTP_OUTGOING_DIR}/${UUID}.log.gz"

# Determine if the time limit has been exceeded
EXCEEDED=\`grep walltime ${USER_NAME}.o* | grep -c "exceeded limit"\`

# Has a segmentation fault occurred?
SEGFAULT=\`grep -c "Segmentation fault" ${USER_NAME}.o*\`

# Did we run out of memory?
BADALLOC=\`grep -c "St9bad_alloc" ${USER_NAME}.o*\`

# Send an email to the user
echo "To: $REQUESTER_EMAIL" > ${UUID}.mail
echo "From: $FROM_EMAIL" >> ${UUID}.mail
if [ \${EXCEEDED} -ne 0 ]; then
    echo "Subject: Duchamp job time limit exceeded (FAILURE)" >> ${UUID}.mail
else
    echo "Subject: Duchamp job completed (FAILURE)" >> ${UUID}.mail
fi
if [ \${SEGFAULT} -ne 0 ]; then
    echo "CC: ${MANAGER_EMAIL}" >> ${UUID}.mail
fi
echo >> ${UUID}.mail
echo "JobId: ${UUID}" >> ${UUID}.mail
echo >> ${UUID}.mail
echo "Job completed with errors. A log of the job execution can be downloaded from the following location:" >> ${UUID}.mail
echo "ftp://${USER_NAME}@ftp.atnf.csiro.au/outgoing/${UUID}.log.gz" >> ${UUID}.mail
echo >> ${UUID}.mail
if [ \${SEGFAULT} -ne 0 ]; then
    echo "A segmentation fault occurred - this message has been CC-ed to the manager of this service for investigation." >> ${UUID}.mail
    echo >> ${UUID}.mail
fi
if [ \${BADALLOC} -ne 0 ]; then
    echo "The job failed with memory allcoation errors, most likely due to it requesting too much memory." >> ${UUID}.mail
    echo >> ${UUID}.mail
fi
echo "============================ Original Job Request Template ============================" >> ${UUID}.mail
cat $JOB_FILE >> ${UUID}.mail

#/usr/sbin/sendmail $REQUESTER_EMAIL < ${UUID}.mail
/usr/sbin/sendmail -t < ${UUID}.mail
EOF

#
# Submit the job
#
NOTOKJOBID=`qsub -W depend=afternotok:${JOBID} ${WORK_DIR}/afternotok.qsub 2>&1`
if [ $? == 0 ]; then
    echo "INFO: Submitted watcher job ${NOTOKJOBID} for request ${UUID}"
else
    echo "ERROR: Failed to submit watcher job. Output: ${NOTOKJOBID}"
fi

# Now the watcher job is submitted, release the main job
qrls $JOBID
if [ $? -ne 0 ]; then
    echo "ERROR: Failed to qrls job ${JOBID}. Output: ${NOTOKJOBID}"
fi
