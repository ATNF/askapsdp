#!/bin/bash -l
##############################################################################
# Measurement Set Averaging (For Gains calibration and Continuum imaging)
##############################################################################

if [ $doAverageMS == true ]; then

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
    
    if [ "${calDepend}" == "" ]; then
	imdepend="--dependency=afterok"
    else
	imdepend=${calDepend}
    fi
    
# Create the sbatch file
    cat > split-coarse.sbatch << EOF
#!/bin/bash
#SBATCH --ntasks=1
#SBATCH --time=08:00:00
##SBATCH --mail-user first.last@csiro.au
#SBATCH --job-name split-coarse
#SBATCH --mail-type=ALL
#SBATCH --export=ASKAP_ROOT,AIPSPATH

mssplit=${mssplit}

cat > ${parsetdir}/mssplit-coarse.in << EOF_INNER
# Input measurement set
# Default: <no default>
vis         = ${scienceMS}

# Output measurement set
# Default: <no default>
outputvis   = ${coarseMS}

# The channel range to split out into its own measurement set
# Can be either a single integer (e.g. 1) or a range (e.g. 1-300). The range
# is inclusive of both the start and end, indexing is one-based. 
# Default: <no default>
channel     = 1-${END_CHANNEL_CREATECOARSE}

# Defines the number of channel to average to form the one output channel
# Default: 1
width       = 54
EOF_INNER

aprun \${mssplit} -c ${parsetdir}/mssplit-coarse.in > ${logdir}/mssplit-coarse.log
EOF

    if [ ! -e ${coarseMS} ]; then
	echo "MS Averaging: Submitting"
	coarseID=`sbatch ${depend} split-coarse.sbatch | awk '{print $4}'`
	calDepend="${calDepend}:${coarseID}"
	imdepend="${imdepend}:${coarseID}"
#    SBATCH_MSSPLIT=`qsubmit split-coarse.sbatch`
#    SBATCH_NODEPS="${SBATCH_NODEPS} ${SBATCH_MSSPLIT}"
#    GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${SBATCH_MSSPLIT}"
    else
	echo "MS Averaging: Skipping - Output already exists"
    fi

fi

