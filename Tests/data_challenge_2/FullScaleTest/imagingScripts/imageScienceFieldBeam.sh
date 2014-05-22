#!/bin/bash -l


image=${imagebase}.BEAM${POINTING}

if [ $doMFS == true ]; then

    mfsParams="# Parameters needed for MFS:
Cimager.Images.${image}.nterms             = 2
Cimager.nworkergroups                           = 3
Cimager.visweights                              = MFS
Cimager.visweights.MFS.reffreq                  = ${CONT_CLEAN_FREQ}"

else

    mfsParams="# Not doing MFS"

fi

slurmtag="cimSci${POINTING}"
sbatchfile=cim_Science_BEAM${POINTING}.sbatch
cat > $sbatchfile <<EOF
#!/bin/bash -l
#SBATCH --time=03:00:00
#SBATCH --ntasks=${CONT_CLEAN_MPPWIDTH}
#SBATCH --ntasks-per-node=${CONT_CLEAN_MPPNPPN}
#SBATCH --job-name ${slurmtag}
#SBATCH --mail-type=ALL
#SBATCH --export=ASKAP_ROOT,AIPSPATH

cim=${cim}

imParset=${parsetdir}/cimScience-BEAM${POINTING}-\${SLURM_JOB_ID}.in
imLogfile=${logdir}/cim-Science-BEAM${POINTING}-\${SLURM_JOB_ID}.log

cat > \${imParset} << EOF_INNER
Cimager.dataset                                 = ${coarseMS}
Cimager.Feed                                    = ${POINTING}
#
# Each worker will read a single channel selection
Cimager.Channels                                = [1, %w]
#
Cimager.Images.Names                            = [${image}]
Cimager.Images.shape                            = [${IMAGING_NUM_PIXELS},${IMAGING_NUM_PIXELS}]
Cimager.Images.cellsize                         = [${IMAGING_CELLSIZE},${IMAGING_CELLSIZE}]
Cimager.Images.${image}.frequency          = [${CONT_CLEAN_FREQ},${CONT_CLEAN_FREQ}]
Cimager.Images.${image}.nchan              = 1
Cimager.Images.${image}.direction          = ${IMAGING_DIRECTION}
Cimager.Images.writeAtMajorCycle                = ${IMAGING_WRITEMAJORCYCLE}
#
${mfsParams}
#
Cimager.gridder.snapshotimaging                 = true
Cimager.gridder.snapshotimaging.wtolerance      = ${IMAGING_WTOL}
Cimager.gridder                                 = ${IMAGING_GRIDDER}
Cimager.gridder.${IMAGING_GRIDDER}.wmax                  = ${IMAGING_WMAX}
Cimager.gridder.${IMAGING_GRIDDER}.nwplanes              = ${IMAGING_NWPLANES}
Cimager.gridder.${IMAGING_GRIDDER}.oversample            = ${IMAGING_OVERSAMPLE}
Cimager.gridder.${IMAGING_GRIDDER}.diameter              = 12m
Cimager.gridder.${IMAGING_GRIDDER}.blockage              = 2m
Cimager.gridder.${IMAGING_GRIDDER}.maxfeeds              = 9
Cimager.gridder.${IMAGING_GRIDDER}.maxsupport            = ${IMAGING_MAXSUP}
Cimager.gridder.${IMAGING_GRIDDER}.variablesupport       = true
Cimager.gridder.${IMAGING_GRIDDER}.offsetsupport         = true
Cimager.gridder.${IMAGING_GRIDDER}.frequencydependent    = true
#
Cimager.solver                                  = Clean
Cimager.solver.Clean.algorithm                  = BasisfunctionMFS
Cimager.solver.Clean.niter                      = 5000
Cimager.solver.Clean.gain                       = 0.5
Cimager.solver.Clean.scales                     = ${IMAGING_CLEANSCALES}
Cimager.solver.Clean.verbose                    = False
Cimager.solver.Clean.tolerance                  = 0.01
Cimager.solver.Clean.weightcutoff               = zero
Cimager.solver.Clean.weightcutoff.clean         = false
Cimager.solver.Clean.psfwidth                   = 512
Cimager.solver.Clean.logevery                   = 100
Cimager.threshold.minorcycle                    = [30%, 0.9mJy]
Cimager.threshold.majorcycle                    = 1mJy
Cimager.ncycles                                 = ${IMAGING_CLEAN_NCYC}

Cimager.preconditioner.Names                    = [Wiener, GaussianTaper]
Cimager.preconditioner.GaussianTaper            = ${IMAGING_GAUSSTAPER}
Cimager.preconditioner.Wiener.robustness        = 0.0
Cimager.preconditioner.Wiener.taper             = 64
#
Cimager.restore                                 = true
Cimager.restore.beam                            = fit
#Cimager.restore.equalise                        = True
#
# Apply calibration
Cimager.calibrate                               = ${doCal}
Cimager.calibaccess                             = parset
Cimager.calibaccess.parset                      = caldata-BEAM${POINTING}.dat
Cimager.calibrate.scalenoise                    = true
Cimager.calibrate.allowflag                     = true
EOF_INNER

aprun -n ${CONT_CLEAN_MPPWIDTH} -N ${CONT_CLEAN_MPPNPPN}  \${cim} -c \${imParset} > \${imLogfile}

EOF

if [ $doSubmit == true ]; then

    if [ "${imdepend}" == "" ]; then
	imdepend="--dependency=afterok"
    fi

    latestID=`sbatch $calDepend $sbatchfile | awk '{print $4}'`
    echo "Running cimager for science field, imaging ${ms} to create ${image}.restored: ID=${latestID}"

fi

