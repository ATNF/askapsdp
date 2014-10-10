#!/usr/bin/env bash
#
# Launches a job to image the current beam of the science
# observation, using the BasisfunctionMFS solver.
#
# (c) Matthew Whiting, CSIRO ATNF, 2014

if [ $DO_CONT_IMAGING == true ]; then

    # Define the Cimager parset
    . ${SCRIPTDIR}/getContinuumCimagerParams.sh

    sbatchfile=$slurms/science_continuumImage_beam$BEAM.sbatch
    cat > $sbatchfile <<EOFOUTER
#!/usr/bin/env bash
#SBATCH --time=12:00:00
#SBATCH --ntasks=${NUM_CPUS_CONTIMG_SCI}
#SBATCH --ntasks-per-node=${CPUS_PER_CORE_CONT_IMAGING}
#SBATCH --job-name=clean${BEAM}
#SBATCH --export=ASKAP_ROOT,AIPSPATH

cd $CWD

# Make a copy of this sbatch file for posterity
sedstr="s/sbatch/\${SLURM_JOB_ID}\.sbatch/g"
cp $sbatchfile \`echo $sbatchfile | sed -e \$sedstr\`

parset=$parsets/science_imaging_beam${BEAM}_\${SLURM_JOB_ID}.in
cat > \$parset <<EOFINNER
${cimagerParams}
#
# Apply calibration
Cimager.calibrate                               = false
EOFINNER

log=$logs/science_imaging_beam${BEAM}_\${SLURM_JOB_ID}.log

aprun -n ${NUM_CPUS_CONTIMG_SCI} -N ${CPUS_PER_CORE_CONT_IMAGING} $cimager -c \$parset > \$log

EOFOUTER

    if [ $SUBMIT_JOBS == true ]; then
	DEP=""
	if [ "$ID_AVERAGE_SCI" != "" ]; then
	    DEP="-d afterok:${ID_AVERAGE_SCI}"
	fi	
	ID_CONTIMG_SCI=`sbatch $DEP $sbatchfile | awk '{print $4}'`
	echo "Make a continuum image for beam $BEAM of the science observation, with job ${ID_CONTIMG_SCI}, and flags \"$DEP\""
	if [ "$FLAG_IMAGING_DEP" == "" ]; then
	    FLAG_IMAGING_DEP="-d afterok:${ID_CONTIMG_SCI}"
	else
	    FLAG_IMAGING_DEP="${FLAG_IMAGING_DEP}:${ID_CONTIMG_SCI}"
	fi
    else
	echo "Would make a continuum image for beam $BEAM of the science observation with slurm file $sbatchfile"
    fi

    echo " "

fi
