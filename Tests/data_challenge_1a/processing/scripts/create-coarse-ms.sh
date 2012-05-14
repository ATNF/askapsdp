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
cat > split_coarse.qsub << EOF
#!/bin/bash
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=1:ncpus=1:mem=2GB:mpiprocs=1
#PBS -l walltime=00:30:00
##PBS -M first.last@csiro.au
#PBS -N split
#PBS -m a
#PBS -j oe

#######
# TO RUN (304 jobs):
#  qsub -J 0-303 split_coarse.qsub
#######

cd \${PBS_O_WORKDIR}

WIDTH=54
STARTCHAN=1
ENDCHAN=16416

RANGE1=\`expr \${PBS_ARRAY_INDEX} \* \${WIDTH} + \${STARTCHAN}\`
RANGE2=\`expr \${RANGE1} + \${WIDTH} - 1\`

cat > ${CONFIGDIR}/mssplit_coarse_\${PBS_ARRAY_INDEX}.in << EOF_INNER
# Input measurement set
# Default: <no default>
vis         = ${INPUT_MS}

# Output measurement set
# Default: <no default>
outputvis   = MS/coarse_chan_\${PBS_ARRAY_INDEX}.ms

# The channel range to split out into its own measurement set
# Can be either a single integer (e.g. 1) or a range (e.g. 1-300). The range
# is inclusive of both the start and end, indexing is one-based. 
# Default: <no default>
channel     = \${RANGE1}-\${RANGE2}

# Defines the number of channel to average to form the one output channel
# Default: 1
width       = \${WIDTH}
EOF_INNER

\${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/mssplit.sh -inputs ${CONFIGDIR}/mssplit_coarse_\${PBS_ARRAY_INDEX}.in > ${LOGDIR}/mssplit_coarse_\${PBS_ARRAY_INDEX}.log
EOF

if [ "${DRYRUN}" == "false" ]; then
    if [ ! -e MS/coarse_chan_0.ms ]; then
        echo "MS Split and Average: Submitting task"
        QSUB_MSSPLIT=`${QSUB_CMD} -h -J 0-303 split_coarse.qsub`
        QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_MSSPLIT}"
    else
        echo "MS Split and Average: Skipping task - Output already exists"
    fi
    if [ "${GLOBAL_DEPEND}" == "" ]; then
	GLOBAL_DEPEND="${QSUB_MSSPLIT}"
    else
	GLOBAL_DEPEND="${GLOBAL_DEPEND}:${QSUB_MSSPLIT}"
    fi

else
    echo "MS Split and Average: Dry Run Only"
fi
