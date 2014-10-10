#!/usr/bin/env bash
#
# Launches a job to extract the appropriate beam from the 1934-638
# observation, then flag the data in two passes, one with a dynamic
# threshold and the second with a flat amplitude cut to remove any
# remaining spikes.
#
# (c) Matthew Whiting, CSIRO ATNF, 2014

sedstr="s/BEAM/${BEAM}/g"
ms=`echo $MS_BASE_1934 | sed -e $sedstr`

if [ "$mslist" == "" ]; then 
    mslist="$ms"
else
    mslist="$mslist,$ms"
fi

sbatchfile=$slurms/split_flag_1934_beam$BEAM.sbatch
cat > $sbatchfile <<EOFOUTER
#!/usr/bin/env bash
#SBATCH --time=12:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name=splitflag${BEAM}
#SBATCH --export=ASKAP_ROOT,AIPSPATH

cd $CWD

# Make a copy of this sbatch file for posterity
sedstr="s/sbatch/\${SLURM_JOB_ID}\.sbatch/g"
cp $sbatchfile \`echo $sbatchfile | sed -e \$sedstr\`

parset=${parsets}/split_1934_beam${BEAM}_\${SLURM_JOB_ID}.in
cat > \$parset <<EOFINNER
# Input measurement set
# Default: <no default>
vis         = ${MS_INPUT_1934}

# Output measurement set
# Default: <no default>
outputvis   = ${ms}

# The channel range to split out into its own measurement set
# Can be either a single integer (e.g. 1) or a range (e.g. 1-300). The range
# is inclusive of both the start and end, indexing is one-based.
# Default: <no default>
channel     = ${CHAN_RANGE_1934}

# Beam selection via beam ID
# Select just a single beam for this obs
beams        = [${BEAM}]

# Scan selection for the 1934-638 observation. Assume the scan ID matches the beam ID
scans        = [${BEAM}]

# Set a larger bucketsize
stman.bucketsize  = 65536
stman.tilenchan   = ${NUM_CHAN_TO_AVERAGE}
EOFINNER

log=logs/split_1934_beam${BEAM}_\${SLURM_JOB_ID}.log

aprun -n 1 -N 1 ${mssplit} -c \${parset} > \${log}

########

parset=parsets/cflag_dynamic_1934_beam${BEAM}_\${SLURM_JOB_ID}.in
cat > \$parset <<EOFINNER
# The path/filename for the measurement set
Cflag.dataset                           = ${ms}

# Amplitude based flagging
Cflag.amplitude_flagger.enable           = true
Cflag.amplitude_flagger.dynamicBounds    = true
Cflag.amplitude_flagger.threshold        = ${FLAG_THRESHOLD_DYNAMIC_1934}
Cflag.amplitude_flagger.integrateSpectra = true
Cflag.amplitude_flagger.integrateSpectra.threshold = ${FLAG_THRESHOLD_DYNAMIC_1934}
EOFINNER

log=${logs}/cflag_dynamic_1934_beam${BEAM}_\${SLURM_JOB_ID}.log

aprun -n 1 -N 1 ${cflag} -c \${parset} > \${log}


parset=parsets/cflag_amp_1934_\${SLURM_JOB_ID}.in
cat > \$parset <<EOFINNER
# The path/filename for the measurement set
Cflag.dataset                           = ${ms}

# Amplitude based flagging
Cflag.amplitude_flagger.enable          = true
Cflag.amplitude_flagger.high            = ${FLAG_THRESHOLD_AMPLITUDE_1934}
Cflag.amplitude_flagger.low             = 0.
EOFINNER


log=logs/cflag_amp_1934_beam${BEAM}_\${SLURM_JOB_ID}.log

aprun -n 1 -N 1 ${cflag} -c \${parset} > \${log}

EOFOUTER

if [ $SUBMIT_JOBS == true ]; then
    ID_FLAG_1934=`sbatch $DEP_START $sbatchfile | awk '{print $4}'`
    echo "Splitting and flagging 1934-638, beam $BEAM with job ${ID_FLAG_1934}"
    if [ "$FLAG_1934_DEP" == "" ]; then
	FLAG_1934_DEP="-d afterok:${ID_FLAG_1934}"
    else
	FLAG_1934_DEP="${FLAG_1934_DEP}:${ID_FLAG_1934}"
    fi
else
    echo "Would run splitting & flagging of 1934-638, beam $BEAM, with slurm file $sbatchfile"
fi

echo " "
