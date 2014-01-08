#!/usr/bin/env bash
##############################################################################
# Measurement Set Averaging (For Gains calibration and Continuum imaging)
##############################################################################

# Create a directory to hold the output measurement sets
AVGMSDIR=MS
if [ ! -d ${AVGMSDIR} ]; then
    mkdir ${AVGMSDIR}
    if [ $? -ne 0 ]; then
        echo "Error: Failed to create directory ${AVGMSDIR}"
        exit 1
    fi
fi

# Create the qsub file
cat > split-coarse.qsub << EOF
#!/bin/bash
##PBS -W group_list=${QUEUEGROUP}
#PBS -l mppwidth=1
#PBS -l walltime=02:00:00
##PBS -M first.last@csiro.au
#PBS -N split-coarse
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

cd \${PBS_O_WORKDIR}

cat > ${CONFIGDIR}/mssplit-coarse.in << EOF_INNER
# Input measurement set
# Default: <no default>
vis         = ${INPUT_MS}

# Output measurement set
# Default: <no default>
outputvis   = MS/coarse_chan.ms

# The channel range to split out into its own measurement set
# Can be either a single integer (e.g. 1) or a range (e.g. 1-300). The range
# is inclusive of both the start and end, indexing is one-based. 
# Default: <no default>
channel     = 1-16416

# Defines the number of channel to average to form the one output channel
# Default: 1
width       = 54
EOF_INNER

aprun \${ASKAP_ROOT}/Code/Components/CP/pipelinetasks/current/apps/mssplit.sh -c ${CONFIGDIR}/mssplit-coarse.in > ${LOGDIR}/mssplit-coarse.log
EOF

if [ ! -e MS/coarse_chan.ms ]; then
    echo "MS Averaging: Submitting"
    QSUB_MSSPLIT=`qsubmit split-coarse.qsub`
    QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_MSSPLIT}"
    GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${QSUB_MSSPLIT}"
else
    echo "MS Averaging: Skipping - Output already exists"
fi
