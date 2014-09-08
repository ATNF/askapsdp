#!/usr/bin/env bash
#
# Launches a job to mosaic all individual beam images to a single
# image. 
#
# (c) Matthew Whiting, ATNF, 2014

if [ $doLinmos == true ]; then

    sbatchfile=$slurms/science_linmos.sbatch
    cat > $sbatchfile <<EOFOUTER
#!/usr/bin/env bash
#SBATCH --time=01:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name=linmos
#SBATCH --export=ASKAP_ROOT,AIPSPATH

cd $CWD

# Make a copy of this sbatch file for posterity
sedstr="s/sbatch/\${SLURM_JOB_ID}\.sbatch/g"
cp $sbatchfile \`echo $sbatchfile | sed -e \$sedstr\`

parset=$parsets/science_linmos_\${SLURM_JOB_ID}.in
log=$logs/science_linmos_\${SLURM_JOB_ID}.log

cat > \$parset << EOFINNER
linmos.names            = [beam${BEAM_MIN}..${BEAM_MAX}]
linmos.findmosaics      = true
linmos.weighttype       = FromPrimaryBeamModel
linmos.weightstate      = Inherent
linmos.feeds.centre     = ${linmosFeedCentre}
linmos.feeds.spacing    = ${linmosBeamSpacing}
${linmosBeams}
linmos.psfref           = ${linmosPSFref}
linmos.nterms           = ${ntermsSci}
EOFINNER

aprun -n 1 -N 1 $linmos -c \$parset > \$log

EOFOUTER

    if [ $doSubmit == true ]; then
	ID_LINMOS_SCI=`sbatch $FLAG_IMAGING_DEP $sbatchfile | awk '{print $4}'`
	echo "Make a mosaic image of the science observation, with job ${ID_LINMOS_SCI}, and flags \"${FLAG_IMAGING_DEP}\""
    else
	echo "Would make a mosaic image  of the science observation with slurm file $sbatchfile"
    fi

    echo " "



fi
