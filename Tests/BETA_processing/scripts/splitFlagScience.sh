#!/usr/bin/env bash
#
# Launches a job to extract the science observation from the
# appropriate measurement set, then flag the data in two passes, one
# with a dynamic threshold and the second with a flat amplitude cut to
# remove any remaining spikes.
#
# (c) Matthew Whiting, CSIRO ATNF, 2014

if [ $DO_FLAG_SCIENCE == true ]; then

    if [ "$SCAN_SELECTION_SCIENCE" == "" ]; then
	scanParam="# No scan selection done"
    else
	scanParam="scans        = [${SCAN_SELECTION_SCIENCE}]"
    fi

    sbatchfile=$slurms/split_flag_science_beam${BEAM}.sbatch
    cat > $sbatchfile <<EOFOUTER
#!/usr/bin/env bash
#SBATCH --time=12:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name=splitflagSci${BEAM}
#SBATCH --export=ASKAP_ROOT,AIPSPATH

cd $CWD

# Make a copy of this sbatch file for posterity
sedstr="s/sbatch/\${SLURM_JOB_ID}\.sbatch/g"
cp $sbatchfile \`echo $sbatchfile | sed -e \$sedstr\`

parset=${parsets}/split_science_beam${BEAM}_\${SLURM_JOB_ID}.in
cat > \$parset <<EOFINNER
# Input measurement set
# Default: <no default>
vis         = ${MS_INPUT_SCIENCE}

# Output measurement set
# Default: <no default>
outputvis   = ${msSci}

# The channel range to split out into its own measurement set
# Can be either a single integer (e.g. 1) or a range (e.g. 1-300). The range
# is inclusive of both the start and end, indexing is one-based.
# Default: <no default>
channel     = ${CHAN_RANGE_SCIENCE}

# Beam selection via beam ID
# Select an individual beam
beams        = [${BEAM}]

# Scan selection for the science observation
$scanParam

# Set a larger bucketsize
stman.bucketsize  = 65536
# Make the tile size 54 channels, as that is what we will average over
stman.tilenchan   = ${NUM_CHAN_TO_AVERAGE}
EOFINNER

log=logs/split_science_beam${BEAM}_\${SLURM_JOB_ID}.log

aprun -n 1 -N 1 ${mssplit} -c \${parset} > \${log}

########

parset=parsets/cflag_dynamic_science_beam${BEAM}_\${SLURM_JOB_ID}.in
cat > \$parset <<EOFINNER
# The path/filename for the measurement set
Cflag.dataset                           = ${msSci}

# Amplitude based flagging with dynamic thresholds
#  This finds a statistical threshold in the spectrum of each
#  time-step, then applies the same threshold level to the integrated
#  spectrum at the end.
Cflag.amplitude_flagger.enable           = true
Cflag.amplitude_flagger.dynamicBounds    = true
Cflag.amplitude_flagger.threshold        = ${FLAG_THRESHOLD_DYNAMIC_SCIENCE}
Cflag.amplitude_flagger.integrateSpectra = true
Cflag.amplitude_flagger.integrateSpectra.threshold = ${FLAG_THRESHOLD_DYNAMIC_SCIENCE}
EOFINNER

log=${logs}/cflag_dynamic_science_beam${BEAM}_\${SLURM_JOB_ID}.log

aprun -n 1 -N 1 ${cflag} -c \${parset} > \${log}


parset=parsets/cflag_amp_science_beam${BEAM}_\${SLURM_JOB_ID}.in
cat > \$parset <<EOFINNER
# The path/filename for the measurement set
Cflag.dataset                           = ${msSci}

# Amplitude based flagging
#   Here we apply a simple cut at a given amplitude level, to remove
#   any remaining spikes
Cflag.amplitude_flagger.enable          = true
Cflag.amplitude_flagger.high            = ${FLAG_THRESHOLD_AMPLITUDE_SCIENCE}
Cflag.amplitude_flagger.low             = 0.
EOFINNER

log=logs/cflag_amp_science_beam${BEAM}_\${SLURM_JOB_ID}.log

aprun -n 1 -N 1 ${cflag} -c \${parset} > \${log}

EOFOUTER

    if [ $SUBMIT_JOBS == true ]; then
	ID_FLAG_SCI=`sbatch $DEP_START $sbatchfile | awk '{print $4}'`
	echo "Splitting and flagging beam ${BEAM} of science observation, with job ${ID_FLAG_SCI}"
    else
	echo "Would run splitting & flagging beam ${BEAM} of science observation with slurm file $sbatchfile"
    fi

    echo " "

fi
