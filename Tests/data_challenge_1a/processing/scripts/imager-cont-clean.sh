#!/usr/bin/env bash
##############################################################################
# Continuum Imaging (Clean)
##############################################################################

cat > cimager-cont-clean.qsub << EOF
#!/bin/bash
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=${CONT_CLEAN_SELECT}
#PBS -l walltime=12:00:00
##PBS -M first.last@csiro.au
#PBS -N cont-clean
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

cd \${PBS_O_WORKDIR}

parset=${CONFIGDIR}/cimager-cont-clean-\${PBS_JOBID}.in
logfile=${LOGDIR}/cimager-cont-clean-\${PBS_JOBID}.log

cat > \$parset << EOF_INNER
Cimager.dataset                                 = MS/coarse_chan_%w.ms
Cimager.nworkergroups                           = 3

Cimager.Images.Names                            = [image.i.clean]
Cimager.Images.shape                            = [${IMAGING_NUM_PIXELS},${IMAGING_NUM_PIXELS}]
Cimager.Images.cellsize                         = [${IMAGING_CELLSIZE},${IMAGING_CELLSIZE}]
Cimager.Images.image.i.clean.frequency          = [${CONT_CLEAN_FREQ},${CONT_CLEAN_FREQ}]
Cimager.Images.image.i.clean.nchan              = 1
Cimager.Images.image.i.clean.direction          = ${IMAGING_DIRECTION}
Cimager.Images.image.i.clean.nterms             = 2
Cimager.Images.writeAtMajorCycle                = true
#
Cimager.visweights                              = MFS
Cimager.visweights.MFS.reffreq                  = 1.270e9
#
Cimager.gridder.snapshotimaging                 = true
Cimager.gridder.snapshotimaging.wtolerance      = ${IMAGING_WTOL}
Cimager.gridder                                 = AWProject
Cimager.gridder.AWProject.wmax                  = ${IMAGING_WMAX}
Cimager.gridder.AWProject.nwplanes              = 99
Cimager.gridder.AWProject.oversample            = 4
Cimager.gridder.AWProject.diameter              = 12m
Cimager.gridder.AWProject.blockage              = 2m
Cimager.gridder.AWProject.maxfeeds              = 36
Cimager.gridder.AWProject.maxsupport            = 512
Cimager.gridder.AWProject.variablesupport       = true
Cimager.gridder.AWProject.offsetsupport         = true
Cimager.gridder.AWProject.frequencydependent    = true
#
Cimager.solver                                  = Clean
Cimager.solver.Clean.algorithm                  = BasisfunctionMFS
Cimager.solver.Clean.niter                      = 5000
Cimager.solver.Clean.gain                       = 0.5
Cimager.solver.Clean.scales                     = [0, 3, 10, 30]
Cimager.solver.Clean.verbose                    = False
Cimager.solver.Clean.tolerance                  = 0.01
Cimager.solver.Clean.weightcutoff               = zero
Cimager.solver.Clean.weightcutoff.clean         = false
Cimager.solver.Clean.psfwidth                   = 512
Cimager.solver.Clean.logevery                   = 100
Cimager.threshold.minorcycle                    = [30%, 0.9mJy]
Cimager.threshold.majorcycle                    = 1mJy
Cimager.ncycles                                 = 5
#
Cimager.preconditioner.Names                    = [Wiener, GaussianTaper]
Cimager.preconditioner.GaussianTaper            = ${IMAGING_GAUSSTAPER}
Cimager.preconditioner.Wiener.robustness        = 0.0
Cimager.preconditioner.Wiener.taper             = 64
#
Cimager.restore                                 = true
Cimager.restore.beam                            = fit
Cimager.restore.equalise                        = ${IMAGING_EQUALISE}
#
# Apply calibration
Cimager.calibrate                               = ${DO_CALIBRATION}
Cimager.calibaccess                             = table
Cimager.calibaccess.table                       = ${CALOUTPUT}
#Cimager.calibaccess                             = parset
#Cimager.calibaccess.parset                      = result.dat
Cimager.calibrate.scalenoise                    = true
Cimager.calibrate.allowflag                     = true
EOF_INNER

mpirun \${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/cimager.sh -c \${parset} > \${logfile}
EOF

# Submit the job
echo "Continuum Imager (Clean): Submitting"

unset DEPENDS
if [ "${QSUB_CAL}" ]; then
    DEPENDS="afterok:${QSUB_CAL}"
    QSUB_CONTCLEAN=`qsubmit cimager-cont-clean.qsub`
elif [ "${QSUB_MSSPLIT}" ]; then
    DEPENDS="afterok:${QSUB_MSSPLIT}"
    QSUB_CONTCLEAN=`qsubmit cimager-cont-clean.qsub`
else
    QSUB_CONTCLEAN=`qsubmit cimager-cont-clean.qsub`
    QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_CONTCLEAN}"
fi

GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${QSUB_CONTCLEAN}"

# Run the analysis script
if [ $DO_ANALYSIS == true ]; then
    unset DEPENDS
    DEPENDS="afterok:${QSUB_CONTCLEAN}"
    CONTINUUMIMAGE=image.i.clean.taylor.0.restored
    . ${SCRIPTDIR}/analysis.sh
fi
