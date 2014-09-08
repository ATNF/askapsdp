#!/usr/bin/env bash
#
# Launches a job to solve for the bandpass calibration. This uses all
# 1934-638 measurement sets after the splitFlag1934.sh jobs have
# completed. The bandpass calibration is done assuming the special
# '1934-638' component.
#
# (c) Matthew Whiting, ATNF, 2014

sbatchfile=$slurms/cbpcalibrator_1934.sbatch
cat > $sbatchfile <<EOF
#!/usr/bin/env bash
#SBATCH --time=12:00:00
#SBATCH --ntasks=343
#SBATCH --ntasks-per-node=20
#SBATCH --job-name=cbpcal
#SBATCH --export=ASKAP_ROOT,AIPSPATH

# Make a copy of this sbatch file for posterity
sedstr="s/sbatch/\${SLURM_JOB_ID}\.sbatch/g"
cp $sbatchfile \`echo $sbatchfile | sed -e \$sedstr\`

parset=${parsets}/cbpcalibrator_1934_\${SLURM_JOB_ID}.in
cat > \$parset <<EOFINNER
Cbpcalibrator.dataset                         = [${mslist}]
Cbpcalibrator.nAnt                            = 6
Cbpcalibrator.nBeam                           = ${nbeam}
Cbpcalibrator.nChan                           = ${nchanCBP}
Cbpcalibrator.refantenna                      = 1
#
Cbpcalibrator.calibaccess                     = table
Cbpcalibrator.calibaccess.table.maxant        = 6
Cbpcalibrator.calibaccess.table.maxbeam       = ${nbeam}
Cbpcalibrator.calibaccess.table.maxchan       = ${nchanCBP}
Cbpcalibrator.calibaccess.table               = ${bandpassParams}
#
Cbpcalibrator.sources.names                   = [field1]
Cbpcalibrator.sources.field1.direction        = ${direction1934}
Cbpcalibrator.sources.field1.components       = src
Cbpcalibrator.sources.src.calibrator          = 1934-638
#
Cbpcalibrator.gridder                         = SphFunc
#
Cbpcalibrator.ncycles                         = ${ncyclesCBPcal}

EOFINNER

log=${logs}/cbpcalibrator_1934_\${SLURM_JOB_ID}.log

aprun -n 343 -N 20 ${cbpcalibrator} -c \${parset} > \${log}

EOF

if [ $doSubmit == true ]; then
    ID_CBPCAL=`sbatch ${FLAG_1934_DEP} $sbatchfile | awk '{print $4}'`
    echo "Calibrating with 1934-638 with job ${ID_CBPCAL}, and flags \"$FLAG_1934_DEP\""
else
    echo "Would find bandpass calibration with slurm file $sbatchfile"
fi

echo " "
