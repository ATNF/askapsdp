#!/usr/bin/env bash
#
# Launches a job to average the measurement set for the current beam
# of the science observation so that it can be imaged by the continuum
# imager. 
#
# (c) Matthew Whiting, CSIRO ATNF, 2014

sedstr="s/\.ms/_averaged\.ms/g"
msSciAv=`echo $msSci | sed -e $sedstr`

if [ $DO_AVERAGE_CHANNELS == true ]; then

    sbatchfile=$slurms/science_average_beam${BEAM}.sbatch
    cat > $sbatchfile <<EOFOUTER
#!/usr/bin/env bash
#SBATCH --time=12:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name=avSci${BEAM}
#SBATCH --export=ASKAP_ROOT,AIPSPATH

cd $CWD

# Make a copy of this sbatch file for posterity
sedstr="s/sbatch/\${SLURM_JOB_ID}\.sbatch/g"
cp $sbatchfile \`echo $sbatchfile | sed -e \$sedstr\`

parset=parsets/science_average_beam${BEAM}_\${SLURM_JOB_ID}.in
cat > \$parset <<EOFINNER
# Input measurement set
# Default: <no default>
vis         = ${msSci}

# Output measurement set
# Default: <no default>
outputvis   = ${msSciAv}

# The channel range to split out into its own measurement set
# Can be either a single integer (e.g. 1) or a range (e.g. 1-300). The range
# is inclusive of both the start and end, indexing is one-based.
# Default: <no default>
channel     = ${CHAN_RANGE_SCIENCE}

# Defines the number of channel to average to form the one output channel
# Default: 1
width       = ${NUM_CHAN_TO_AVERAGE}

EOFINNER

log=logs/science_average_beam${BEAM}_\${SLURM_JOB_ID}.log

aprun -n 1 -N 1 $mssplit -c \${parset} > \${log}

EOFOUTER

    if [ $SUBMIT_JOBS == true ]; then
	DEP=""
	if [ "$ID_CCALAPPLY_SCI" != "" ]; then
	    DEP="-d afterok:${ID_CCALAPPLY_SCI}"
	fi	
	ID_AVERAGE_SCI=`sbatch $DEP $sbatchfile | awk '{print $4}'`
	echo "Averaging beam ${BEAM} of the science observation, with job ${ID_AVERAGE_SCI}, and flags \"$DEP\""
    else
	echo "Would average beam ${BEAM} of the science observation with slurm file $sbatchfile"
    fi

    echo " "

fi
