##############################################################################
# Spectral Line Imaging
##############################################################################

# Create a directory to hold the output measurement sets
MSDIR=MSFINE
if [ ! -d ${MSDIR} ]; then
    mkdir ${MSDIR}
    if [ $? -ne 0 ]; then
        echo "Error: Failed to create directory ${MSDIR}"
        exit 1
    fi
fi

#
# Create the qsub file to image each channel individually
#
cat > cimager-spectral-line.qsub << EOF
#!/bin/bash
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=1:ncpus=1:mem=3GB:mpiprocs=1
#PBS -l walltime=03:00:00
##PBS -M first.last@csiro.au
#PBS -N sl-img
#PBS -m a
#PBS -j oe

#######
# TO RUN (16200 jobs):
#  qsub -J 0-16199 cimager-spectral-line.qsub
#######

cd \${PBS_O_WORKDIR}

CHAN=\$((\${PBS_ARRAY_INDEX} + 1))

cat > ${CONFIGDIR}/mssplit_fine_\${PBS_ARRAY_INDEX}.in << EOF_INNER
# Input measurement set
# Default: <no default>
vis         = ${INPUT_MS}

# Output measurement set
# Default: <no default>
outputvis   = ${MSDIR}/fine_chan_\${PBS_ARRAY_INDEX}.ms

# The channel range to split out into its own measurement set
# Can be either a single integer (e.g. 1) or a range (e.g. 1-300). The range
# is inclusive of both the start and end, indexing is one-based. 
# Default: <no default>
channel     = \${CHAN}

# Defines the number of channel to average to form the one output channel
# Default: 1
width       = 1
EOF_INNER

cat > ${CONFIGDIR}/cimager_spectral_\${PBS_ARRAY_INDEX}.in << EOF_INNER
Cimager.dataset                                 = ${MSDIR}/fine_chan_\${PBS_ARRAY_INDEX}.ms

Cimager.Images.Names                            = [image.i.spectral.\${PBS_ARRAY_INDEX}]
Cimager.Images.shape                            = [3584,3584]
Cimager.Images.cellsize                         = [10arcsec, 10arcsec]
Cimager.Images.image.i.spectral.\${PBS_ARRAY_INDEX}.frequency  = [1.420e9,1.420e9]
Cimager.Images.image.i.spectral.\${PBS_ARRAY_INDEX}.nchan      = 1
Cimager.Images.image.i.spectral.\${PBS_ARRAY_INDEX}.direction  = [12h30m00.00, -45.00.00.00, J2000]
#
Cimager.gridder.snapshotimaging                 = true
Cimager.gridder.snapshotimaging.wtolerance      = 2000
Cimager.gridder                                 = AWProject
Cimager.gridder.AWProject.wmax                  = 2000
Cimager.gridder.AWProject.nwplanes              = 7
Cimager.gridder.AWProject.oversample            = 4
Cimager.gridder.AWProject.diameter              = 12m
Cimager.gridder.AWProject.blockage              = 2m
Cimager.gridder.AWProject.maxfeeds              = 36
Cimager.gridder.AWProject.maxsupport            = 2048
Cimager.gridder.AWProject.variablesupport       = true
Cimager.gridder.AWProject.offsetsupport         = true
Cimager.gridder.AWProject.frequencydependent    = true
#
Cimager.solver                                  = Dirty
Cimager.solver.Dirty.tolerance                  = 0.1
Cimager.solver.Dirty.verbose                    = True
Cimager.ncycles                                 = 0

Cimager.preconditioner.Names                    = None

# Apply calibration
#Cimager.calibrate                               = true
Cimager.calibrate                               = false
Cimager.calibaccess                             = table
Cimager.calibaccess.table                       = ${CALOUTPUT}
EOF_INNER

LOGFILE=${LOGDIR}/cimager_spectral_\${PBS_ARRAY_INDEX}.log

# First split the big measurement set
\${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/mssplit.sh -inputs ${CONFIGDIR}/mssplit_fine_\${PBS_ARRAY_INDEX}.in > \${LOGFILE}
if [ \$? -ne 0 ]; then
    echo "Error: mssplit returned non-zero error code"
    exit 1
fi

# Now run the cimager
mpirun \${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/cimager.sh -inputs ${CONFIGDIR}/cimager_spectral_\${PBS_ARRAY_INDEX}.in >> \${LOGFILE}

# Finally delete the temporary split-off measurement set
rm -rf ${MSDIR}/fine_chan_\${PBS_ARRAY_INDEX}.ms
EOF

#
# Merge imaged slices into a cube
#
cat > cubemerge-spectral-line.qsub << EOF
#!/bin/bash
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=1:ncpus=1:mem=4GB:mpiprocs=1
#PBS -l walltime=12:00:00
##PBS -M first.last@csiro.au
#PBS -N sl-mkcube
#PBS -m a
#PBS -j oe

cd \${PBS_O_WORKDIR}

OUTPUTCUBE=imagecube.i.spectral
INPUTSLICES=\`find . -maxdepth 1 -name 'residual.i.spectral.*' | sort\`
if [ ! \${INPUTSLICES} ]; then
    echo "Error: Could not find input image slices"
    exit 1
fi

rm -rf \${OUTPUTCUBE}
\${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/cubemerge.sh \${INPUTSLICES} \${OUTPUTCUBE}
EOF

if [ ! $DRYRUN ]; then
        echo "Spectral Line Imaging: Submitting tasks"
        QSUB_SPECTRAL1=`${QSUB_CMD} -N sl-img1 -h -J 0-8099 cimager-spectral-line.qsub`
        QSUB_SPECTRAL2=`${QSUB_CMD} -N sl-img2 -h -J 8100-16199 cimager-spectral-line.qsub`
        QSUB_CUBEMERGE=`${QSUB_CMD} -W depend=afterok:${QSUB_SPECTRAL1},afterok:${QSUB_SPECTRAL2} cubemerge-spectral-line.qsub`
        QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_SPECTRAL1} ${QSUB_SPECTRAL2}"
else
    echo "Spectral Line Imaging: Dry Run Only"
fi
