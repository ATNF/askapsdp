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

    # Will contain a small number of large files accessed
    # by multiple nodes, so stripe the files over a few
    # storage servers
    lfs setstripe -c 8 MS
fi

# Create the qsub file
cat > split-coarse.sbatch << EOF
#!/bin/bash
#SBATCH --ntasks=1
#SBATCH --time=04:00:00
##SBATCH --mail-user first.last@csiro.au
#SBATCH --job-name split-coarse
#SBATCH --mail-type=ALL
#SBATCH --export=ASKAP_ROOT,AIPSPATH

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
channel     = 1-${END_CHANNEL_CREATECOARSE}

# Defines the number of channel to average to form the one output channel
# Default: 1
width       = 54
EOF_INNER

aprun \${ASKAP_ROOT}/Code/Components/CP/pipelinetasks/current/apps/mssplit.sh -c ${CONFIGDIR}/mssplit-coarse.in > ${LOGDIR}/mssplit-coarse.log
EOF

if [ ! -e MS/coarse_chan.ms ]; then
    echo "MS Averaging: Submitting"
    SBATCH_MSSPLIT=`qsubmit split-coarse.sbatch`
    SBATCH_NODEPS="${SBATCH_NODEPS} ${SBATCH_MSSPLIT}"
    GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${SBATCH_MSSPLIT}"
else
    echo "MS Averaging: Skipping - Output already exists"
fi
