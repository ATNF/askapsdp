#!/usr/bin/env bash
#
# Launches a job to image the current beam of the science
# observation, using the BasisfunctionMFS solver.
#
# (c) Matthew Whiting, ATNF, 2014

if [ $doContinuumImaging == true ]; then

    imageBase=${sciContImageBase}.beam${BEAM}

    shapeDefinition="# Leave shape definition to advise"
    if [ $pixsizeCont -gt 0 ]; then
	shapeDefinition="Cimager.Images.shape                            = [${pixsizeCont}, ${pixsizeCont}]"
    fi
    cellsizeDefinition="# Leave cellsize definition to advise"
    if [ $cellsizeCont -gt 0 ]; then
	cellsizeDefinition="Cimager.Images.cellsize                         = [${cellsizeCont}arcsec, ${cellsizeCont}arcsec]"
    fi

    sbatchfile=$slurms/science_continuumImage_beam$BEAM.sbatch
    cat > $sbatchfile <<EOFOUTER
#!/usr/bin/env bash
#SBATCH --time=12:00:00
#SBATCH --ntasks=${NUM_CPUS_CONTIMG_SCI}
#SBATCH --ntasks-per-node=${CPUS_PER_CORE_CONTIMG_SCI}
#SBATCH --job-name=clean${BEAM}
#SBATCH --export=ASKAP_ROOT,AIPSPATH

cd $CWD

# Make a copy of this sbatch file for posterity
sedstr="s/sbatch/\${SLURM_JOB_ID}\.sbatch/g"
cp $sbatchfile \`echo $sbatchfile | sed -e \$sedstr\`

parset=$parsets/science_imaging_beam${BEAM}_\${SLURM_JOB_ID}.in
cat > \$parset <<EOFINNER
Cimager.dataset                                 = ${msSciAv}
#
# Each worker will read a single channel selection
Cimager.Channels                                = [1, %w]
#
Cimager.Images.Names                            = [image.${imageBase}]
${shapeDefinition}
${cellsizeDefinition}
# This is how many channels to write to the image - just a single one for continuum
Cimager.Images.image.${imageBase}.nchan         = 1
#
# The following are needed for MFS clean
# This one says how many Taylor terms we need
Cimager.Images.image.${imageBase}.nterms        = ${ntermsSci}
# This one assigns one worker for each of the Taylor terms
Cimager.nworkergroups                           = ${nworkergroupsSci}
# This tells the gridder to weight the visibilities appropriately
Cimager.visweights                              = MFS
# This is the reference frequency - it should lie in your frequency range (ideally in the middle)
Cimager.visweights.MFS.reffreq                  = ${freqContSci}
#
# This defines the parameters for the gridding.
Cimager.gridder.snapshotimaging                 = true
Cimager.gridder.snapshotimaging.wtolerance      = 2600
Cimager.gridder                                 = WProject
Cimager.gridder.WProject.wmax                   = 2600
Cimager.gridder.WProject.nwplanes               = 99
Cimager.gridder.WProject.oversample             = 4
Cimager.gridder.WProject.diameter               = 12m
Cimager.gridder.WProject.blockage               = 2m
Cimager.gridder.WProject.maxfeeds               = 9
Cimager.gridder.WProject.maxsupport             = 512
Cimager.gridder.WProject.variablesupport        = true
Cimager.gridder.WProject.offsetsupport          = true
Cimager.gridder.WProject.frequencydependent     = true
#
Cimager.solver                                  = Clean
Cimager.solver.Clean.algorithm                  = BasisfunctionMFS
Cimager.solver.Clean.niter                      = 500
Cimager.solver.Clean.gain                       = 0.5
Cimager.solver.Clean.scales                     = [0, 3, 10]
Cimager.solver.Clean.verbose                    = False
Cimager.solver.Clean.tolerance                  = 0.01
Cimager.solver.Clean.weightcutoff               = zero
Cimager.solver.Clean.weightcutoff.clean         = false
Cimager.solver.Clean.psfwidth                   = 512
Cimager.solver.Clean.logevery                   = 50
Cimager.threshold.minorcycle                    = [30%, 0.9mJy]
Cimager.threshold.majorcycle                    = 1mJy
Cimager.ncycles                                 = 2
Cimager.Images.writeAtMajorCycle                = false
#
Cimager.preconditioner.Names                    = [Wiener, GaussianTaper]
Cimager.preconditioner.GaussianTaper            = [30arcsec, 30arcsec, 0deg]
Cimager.preconditioner.Wiener.robustness        = 0.5
#
Cimager.restore                                 = true
Cimager.restore.beam                            = ${restoringBeamCont}
#
# Apply calibration
Cimager.calibrate                               = false
EOFINNER

log=$logs/science_imaging_beam${BEAM}_\${SLURM_JOB_ID}.log

aprun -n ${NUM_CPUS_CONTIMG_SCI} -N ${CPUS_PER_CORE_CONTIMG_SCI} $cimager -c \$parset > \$log

EOFOUTER

    if [ $doSubmit == true ]; then
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
