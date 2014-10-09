#!/usr/bin/env bash
#
# Launches a job to apply the bandpass solution to the measurement set 
#
# (c) Matthew Whiting, CSIRO ATNF, 2014

if [ $doApplyBandpass == true ]; then

    sbatchfile=$slurms/ccalapply_science_beam$BEAM.sbatch
    cat > $sbatchfile <<EOFOUTER
#!/usr/bin/env bash
#SBATCH --time=12:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name=calapply${BEAM}
#SBATCH --export=ASKAP_ROOT,AIPSPATH

# Make a copy of this sbatch file for posterity
sedstr="s/sbatch/\${SLURM_JOB_ID}\.sbatch/g"
cp $sbatchfile \`echo $sbatchfile | sed -e \$sedstr\`

parset=${parsets}/ccalapply_bp_\${SLURM_JOB_ID}.in
cat > \$parset << EOFINNER
Ccalapply.dataset                             = ${msSci}
#
# Allow flagging of vis if inversion of Mueller matrix fails
Ccalapply.calibrate.allowflag                 = true
#
Ccalapply.calibaccess                     = table
Ccalapply.calibaccess.table.maxant        = 6
Ccalapply.calibaccess.table.maxbeam       = ${nbeam}
Ccalapply.calibaccess.table.maxchan       = ${nchanSci}
Ccalapply.calibaccess.table               = ${bandpassParams}

EOFINNER

log=${logs}/ccalapply_bp_\${SLURM_JOB_ID}.log

aprun -n 1 -N 1 ${ccalapply} -c \$parset > \$log

EOFOUTER

    if [ $doSubmit == true ]; then
	DEP=""
	if [ "$ID_FLAG_SCI" != "" ] || [ "$ID_CBPCAL" != "" ]; then
	    DEP="-d afterok"
	    if [ "$ID_FLAG_SCI" != "" ]; then
		DEP="${DEP}:${ID_FLAG_SCI}"
	    fi
	    if [ "$ID_CBPCAL" != "" ]; then
		DEP="${DEP}:${ID_CBPCAL}"
	    fi
	fi	
	ID_CCALAPPLY_SCI=`sbatch $DEP $sbatchfile | awk '{print $4}'`
	echo "Applying bandpass calibration to science observation, with job ${ID_CCALAPPLY_SCI}, and flags \"$DEP\""
    else
	echo "Would apply bandpass calibration to science observation with slurm file $sbatchfile"
    fi

    echo " "

fi
