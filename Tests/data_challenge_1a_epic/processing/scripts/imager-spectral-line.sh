#!/usr/bin/env bash
##############################################################################
# Spectral Line Imaging
##############################################################################

# Create a work dir directory and one to hold the output measurement sets
SL_WORK_DIR=sl-work-dir
if [ ! -d ${SL_WORK_DIR} ]; then
    mkdir ${SL_WORK_DIR}
    if [ $? -ne 0 ]; then
        echo "Error: Failed to create directory ${SL_WORK_DIR}"
        exit 1
    fi
    mkdir ${SL_WORK_DIR}/MS
    if [ $? -ne 0 ]; then
        echo "Error: Failed to create directory ${SL_WORK_DIR}/MS"
        exit 1
    fi
fi

mv askap.log_cfg ${SL_WORK_DIR}

#
# Create the qsub file to image each channel individually
#
cat > cimager-spectral-line.qsub << EOF
#!/bin/bash
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=1:ncpus=1:mem=6GB:mpiprocs=1
#PBS -l walltime=04:00:00
##PBS -M first.last@csiro.au
#PBS -N sl-img
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

#######
# TO RUN (${NUM_WORKERS_SPECTRAL_CUBE} jobs):
#  qsub -J ${QSUB_RANGE_SPECTRAL_CUBE_FULL} cimager-spectral-line.qsub
#######

cd \${PBS_O_WORKDIR}/${SL_WORK_DIR}

CHAN=\$((\${PBS_ARRAY_INDEX} + 1))

MSSPLITPARSET=../${CONFIGDIR}/mssplit_fine_\${PBS_ARRAY_INDEX}.in
CIMAGERPARSET=../${CONFIGDIR}/cimager_spectral_\${PBS_ARRAY_INDEX}.in

cat > \${MSSPLITPARSET} << EOF_INNER
# Input measurement set
# Default: <no default>
vis         = ../${INPUT_MS}

# Output measurement set
# Default: <no default>
outputvis   = MS/fine_chan_\${PBS_ARRAY_INDEX}.ms

# The channel range to split out into its own measurement set
# Can be either a single integer (e.g. 1) or a range (e.g. 1-300). The range
# is inclusive of both the start and end, indexing is one-based. 
# Default: <no default>
channel     = \${CHAN}

# Defines the number of channel to average to form the one output channel
# Default: 1
width       = 1
EOF_INNER

basefreq=${SPECTRAL_CUBE_FREQ_ZERO_CHAN}
dfreq=-18.5185185e3
freq=\`echo \${basefreq} \${dfreq} \${PBS_ARRAY_INDEX} | awk '{printf "%8.6e",\$1+\$2*\$3}'\`

cat > \${CIMAGERPARSET} << EOF_INNER
Cimager.dataset                                 = MS/fine_chan_\${PBS_ARRAY_INDEX}.ms

Cimager.Images.Names                            = [image.i.spectral.\${PBS_ARRAY_INDEX}]
Cimager.Images.shape                            = [${IMAGING_NUM_PIXELS},${IMAGING_NUM_PIXELS}]
Cimager.Images.cellsize                         = [${IMAGING_CELLSIZE},${IMAGING_CELLSIZE}]
Cimager.Images.image.i.spectral.\${PBS_ARRAY_INDEX}.frequency  = [\${freq}, \${freq}]
Cimager.Images.image.i.spectral.\${PBS_ARRAY_INDEX}.nchan      = 1
Cimager.Images.image.i.spectral.\${PBS_ARRAY_INDEX}.direction  = ${IMAGING_DIRECTION}
#
Cimager.gridder.snapshotimaging                 = true
Cimager.gridder.snapshotimaging.wtolerance      = ${IMAGING_WTOL}
Cimager.gridder                                 = AWProject
Cimager.gridder.AWProject.wmax                  = ${IMAGING_WMAX}
Cimager.gridder.AWProject.nwplanes              = 129
Cimager.gridder.AWProject.oversample            = 4
Cimager.gridder.AWProject.diameter              = 12m
Cimager.gridder.AWProject.blockage              = 2m
Cimager.gridder.AWProject.maxfeeds              = 36
Cimager.gridder.AWProject.maxsupport            = ${IMAGING_MAXSUP}
Cimager.gridder.AWProject.variablesupport       = true
Cimager.gridder.AWProject.offsetsupport         = true
Cimager.gridder.AWProject.frequencydependent    = true
#
Cimager.solver                                  = Dirty
Cimager.solver.Dirty.tolerance                  = 0.1
Cimager.solver.Dirty.verbose                    = True
Cimager.ncycles                                 = 0

Cimager.preconditioner.Names                    = None
#
Cimager.restore                                 = true
Cimager.restore.beam                            = fit
#Cimager.restore.equalise                        = ${IMAGING_EQUALISE}
#
# Apply calibration
Cimager.calibrate                               = ${DO_CALIBRATION}
Cimager.calibaccess                             = table
Cimager.calibaccess.table                       = ${CALOUTPUT}
Cimager.calibrate.scalenoise                    = true
Cimager.calibrate.allowflag                     = true
EOF_INNER

LOGFILE=../${LOGDIR}/cimager_spectral_\${PBS_ARRAY_INDEX}.log

# First split the big measurement set
\${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/mssplit.sh -c \${MSSPLITPARSET} > \${LOGFILE}
ERR=\$?
if [ \${ERR} -ne 0 ]; then
    echo "Error: mssplit returned error code \${ERR}"
    exit 1
fi

# Now run the cimager
mpirun \${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/cimager.sh -c \${CIMAGERPARSET} >> \${LOGFILE}
ERR=\$?
if [ \${ERR} -ne 0 ]; then
    echo "Error: cimager returned error code \${ERR}"
    exit 1
fi

# Finally delete the temporary split-off measurement set
rm -rf MS/fine_chan_\${PBS_ARRAY_INDEX}.ms
EOF

# Build dependencies
unset DEPENDS
if [ "${QSUB_CAL}" ]; then
    DEPENDS="afterok:${QSUB_CAL}"
fi

# Submit the jobs
echo "Spectral Line Imaging: Submitting"
QSUB_SPECTRAL1=`qsubmit -N sl-img1 -J ${QSUB_RANGE_SPECTRAL_CUBE_1} cimager-spectral-line.qsub`
QSUB_SPECTRAL2=`qsubmit -N sl-img2 -J ${QSUB_RANGE_SPECTRAL_CUBE_2} cimager-spectral-line.qsub`
GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${QSUB_SPECTRAL1} ${QSUB_SPECTRAL2}"

if [ ! "${DEPENDS}" ]; then
    QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_SPECTRAL1} ${QSUB_SPECTRAL2}"
fi

# This becomes a dependency for the makecube jobs
DEPENDS="afterok:${QSUB_SPECTRAL1},afterok:${QSUB_SPECTRAL2}"

# Run makecube using the make-spectral-cube.qsub script
DODELETE=true
FIRSTCH=0
FINALCH=${SPECTRAL_CUBE_FINALCH}
RESTFREQ="HI"

IMAGESUFFIX=".restored"
IMAGEPREFIX="${SL_WORK_DIR}/image.i.spectral."
OUTPUTCUBE=image.i.cube.spectral.restored
. ${SCRIPTDIR}/make-spectral-cube.sh

IMAGESUFFIX=""
OUTPUTCUBE=image.i.cube.spectral
. ${SCRIPTDIR}/make-spectral-cube.sh

IMAGEPREFIX="${SL_WORK_DIR}/psf.i.spectral."
OUTPUTCUBE=psf.i.cube.spectral
. ${SCRIPTDIR}/make-spectral-cube.sh

IMAGEPREFIX="${SL_WORK_DIR}/sensitivity.i.spectral."
OUTPUTCUBE=sensitivity.i.cube.spectral
. ${SCRIPTDIR}/make-spectral-cube.sh

IMAGEPREFIX="${SL_WORK_DIR}/weights.i.spectral."
OUTPUTCUBE=weights.i.cube.spectral
. ${SCRIPTDIR}/make-spectral-cube.sh
