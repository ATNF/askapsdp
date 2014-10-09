#!/usr/bin/env bash
#
# Launches a job to image the current beam of the science
# observation, using the BasisfunctionMFS solver.
#
# (c) Matthew Whiting, ATNF, 2014

if [ $doContinuumImaging == true ] && [ $doSelfcal == true ]; then

    imageBase=${sciContImageBase}.beam${BEAM}

    shapeDefinition="# Leave shape definition to advise"
    if [ $pixsizeCont -gt 0 ]; then
	shapeDefinition="Cimager.Images.shape                            = [${pixsizeCont}, ${pixsizeCont}]"
    fi
    cellsizeDefinition="# Leave cellsize definition to advise"
    if [ $cellsizeCont -gt 0 ]; then
	cellsizeDefinition="Cimager.Images.cellsize                         = [${cellsizeCont}arcsec, ${cellsizeCont}arcsec]"
    fi

    if [ $NUM_CPUS_CONTIMG_SCI -lt 19 ]; then
	NUM_CPUS_SELFCAL=19
    else
	NUM_CPUS_SELFCAL=$NUM_CPUS_CONTIMG_SCI
    fi

    NPROCS_SELAVY=`echo $selfcalNsubx $selfcalNsuby | awk '{print $1*$2+1}'`
    if [ ${CPUS_PER_CORE_CONTIMG_SCI} -lt ${NPROCS_SELAVY} ]; then
	CPUS_PER_CORE_SELFCAL=${CPUS_PER_CORE_CONTIMG_SCI}
    else
	CPUS_PER_CORE_SELFCAL=${NPROCS_SELAVY}
    fi

    sbatchfile=$slurms/science_continuumImageSelfcal_beam$BEAM.sbatch
    cat > $sbatchfile <<EOFOUTER
#!/usr/bin/env bash
#SBATCH --time=12:00:00
#SBATCH --ntasks=${NUM_CPUS_SELFCAL}
#SBATCH --ntasks-per-node=${CPUS_PER_CORE_CONTIMG_SCI}
#SBATCH --job-name=cleanSC${BEAM}
#SBATCH --export=ASKAP_ROOT,AIPSPATH

cd $CWD

# Make a copy of this sbatch file for posterity
sedstr="s/sbatch/\${SLURM_JOB_ID}\.sbatch/g"
cp $sbatchfile \`echo $sbatchfile | sed -e \$sedstr\`

caldir=selfCal_beam${BEAM}
mkdir -p \$caldir

copyImages=${selfcalKeepImages}

for((LOOP=0;LOOP<=${numLoopsSelfcal};LOOP++)); do

    loopdir=\${caldir}/Loop\${LOOP}
    sources=sources_loop\${LOOP}.in
    caldata=caldata_loop\${LOOP}.tab

    if [ \${LOOP} -gt 0 ]; then
        mkdir -p \${loopdir}
        calparams="# Self-calibration using the recently-generated cal table
Cimager.calibrate                           = true
Cimager.calibaccess                         = table
Cimager.calibaccess.table                   = \${loopdir}/\${caldata}
Cimager.calibaccess.table.maxant            = 6
Cimager.calibaccess.table.maxbeam           = 9
Cimager.calibaccess.table.maxchan           = 30
Cimager.calibaccess.table.reuse             = false
"
    else
        calparams="# No self-calibration as it is the first time around the loop
Cimager.calibrate                               = false
"
    fi

    parset=$parsets/science_imagingSelfcal_beam${BEAM}_\${SLURM_JOB_ID}_LOOP\${LOOP}.in
    log=$logs/science_imagingSelfcal_beam${BEAM}_\${SLURM_JOB_ID}_LOOP\${LOOP}.log

    cat > \$parset <<EOFINNER
##########
## Continuum imaging with cimager
##
Cimager.dataset                                 = ${msSciAv}
#
# Each worker will read a single channel selection
Cimager.Channels                                = [1, %w]
#
Cimager.Images.Names                            = [image.${imageBase}]
${shapeDefinition}
${cellsizeDefinition}
# This is how many channels to write to the image - just a single one for continuum
Cimager.Images.image.${imageBase}.nchan        = 1
#
# The following are needed for MFS clean
# This one says how many Taylor terms we need
Cimager.Images.image.${imageBase}.nterms        = ${ntermsSci}
# This one assigns one worker for each of the Taylor terms
Cimager.nworkergroups                           = ${nworkergroupsSci}
# This tells the gridder to weight the visibilities appropriately
#Cimager.visweights                             = MFS
# This is the reference frequency - it should lie in your frequency range (ideally in the middle)
#Cimager.visweights.MFS.reffreq                 = ${freqContSci}
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
\${calparams}
#
EOFINNER
    if [ \${LOOP} -gt 0 ]; then
            cat >> \$parset <<EOFINNER
##########
## Shallow source-finding with selavy
##
# The image to be searched
Selavy.image                                    = ${CWD}/image.${imageBase}.taylor.0.restored
#
# This is how we divide it up for distributed processing, with the
#  number of subdivisions in each direction, and the size of the
#  overlap region in pixels
Selavy.nsubx                                    = ${selfcalNsubx}
Selavy.nsuby                                    = ${selfcalNsuby}
Selavy.overlapx                                 = 50
Selavy.overlapy                                 = 50
#
# The search threshold, in units of sigma
Selavy.snrCut                                   = ${selavySNRselfcal}
# Grow the detections to a secondary threshold
Selavy.flagGrowth                               = true
Selavy.growthCut                                = 5
#
# Turn on the variable threshold option
Selavy.VariableThreshold                        = true
Selavy.VariableThreshold.boxSize                = 50
Selavy.VariableThreshold.ThresholdImageName     = detThresh.img
Selavy.VariableThreshold.NoiseImageName         = noiseMap.img
Selavy.VariableThreshold.AverageImageName       = meanMap.img
Selavy.VariableThreshold.SNRimageName           = snrMap.img
#
# Parameters to switch on and control the Gaussian fitting
Selavy.Fitter.doFit                             = true
# Fit all 6 parameters of the Gaussian
Selavy.Fitter.fitTypes                          = [full]
# Limit the number of Gaussians to 1
Selavy.Fitter.maxNumGauss = 1
# Do not use the number of initial estimates to determine how many Gaussians to fit
Selavy.Fitter.numGaussFromGuess = false
# The fit may be a bit poor, so increase the reduced-chisq threshold
Selavy.Fitter.maxReducedChisq = 10.
#
# Allow islands that are slightly separated to be considered a single 'source'
Selavy.flagAdjacent = false
# The separation in pixels for islands to be considered 'joined'
Selavy.threshSpatial = 7
#
# Saving the fitted components to a parset for use by ccalibrator
Selavy.outputComponentParset                    = \${sources}
# Only use the brightest components in the parset
Selavy.outputComponentParset.maxNumComponents   = 10
#
# Size criteria for the final list of detected islands
Selavy.minPix                                   = 3
Selavy.minVoxels                                = 3
Selavy.minChannels                              = 1
#
# How the islands are sorted in the final catalogue - by
#  integrated flux in this case
Selavy.sortingParam                             = -iflux
#
##########
## Calibration using selavy's component parset
##
# parameters for calibrator
Ccalibrator.dataset                             = ${CWD}/${msSciAv}
Ccalibrator.nAnt                                = 6
Ccalibrator.nBeam                               = 1
Ccalibrator.solve                               = antennagains
Ccalibrator.interval                            = ${intervalSelfcal}
#
Ccalibrator.calibaccess                         = table
Ccalibrator.calibaccess.table                   = \${caldata}
Ccalibrator.calibaccess.table.maxant            = 6
Ccalibrator.calibaccess.table.maxbeam           = 9
Ccalibrator.calibaccess.table.maxchan           = 30
Ccalibrator.calibaccess.table.reuse             = false
#
Ccalibrator.sources.definition                  = \${sources}
#
Ccalibrator.gridder.snapshotimaging             = true
Ccalibrator.gridder.snapshotimaging.wtolerance  = 2600
Ccalibrator.gridder                             = WProject
#
Ccalibrator.ncycles                             = 25

EOFINNER
    fi

    echo "=== Continuum imaging with Self-calibration, for beam ${BEAM}, self-cal loop \${LOOP} ===" > \$log

    # Other than for the first loop, run selavy to extract the
    #  component parset and use it to calibrate
    if [ \${LOOP} -gt 0 ]; then
        cd \${loopdir}
        ln -s ${CWD}/${logs} .
        ln -s ${CWD}/${parsets} .

        echo "--- Source finding with $selavy ---" >> \$log
        aprun -n ${NPROCS_SELAVY} -N ${CPUS_PER_CORE_SELFCAL} $selavy -c \$parset >> \$log
        err=\$?
        if [ \$err != 0 ]; then
            exit \$err
        fi

        echo "--- Calibration with $ccalibrator ---" >> \$log
        aprun -n 1 -N 1 $ccalibrator -c \$parset >> \$log
        err=\$?
        if [ \$err != 0 ]; then
            exit \$err
        fi

        # Keep a backup of the intermediate images, prior to re-imaging.
        if [ \${copyImages} == true ]; then
            mv ${CWD}/*${imageBase}* .
        fi

        cd $CWD
    fi

    # Run the imager, calibrating if not the first time.
    echo "--- Imaging with $cimager ---" >> \$log
    aprun -n ${NUM_CPUS_CONTIMG_SCI} -N ${CPUS_PER_CORE_CONTIMG_SCI} $cimager -c \$parset >> \$log
    err=\$?
    if [ \$err != 0 ]; then
        exit \$err
    fi

done

EOFOUTER

    if [ $doSubmit == true ]; then
	DEP=""
	if [ "$ID_AVERAGE_SCI" != "" ]; then
	    DEP="-d afterok:${ID_AVERAGE_SCI}"
	fi	
	ID_CONTIMG_SCI=`sbatch $DEP $sbatchfile | awk '{print $4}'`
	echo "Make a self-calibrated continuum image for beam $BEAM of the science observation, with job ${ID_CONTIMG_SCI}, and flags \"$DEP\""
	if [ "$FLAG_IMAGING_DEP" == "" ]; then
	    FLAG_IMAGING_DEP="-d afterok:${ID_CONTIMG_SCI}"
	else
	    FLAG_IMAGING_DEP="${FLAG_IMAGING_DEP}:${ID_CONTIMG_SCI}"
	fi
    else
	echo "Would make a self-calibrated continuum image for beam $BEAM of the science observation with slurm file $sbatchfile"
    fi

    echo " "

fi
