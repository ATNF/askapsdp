#!/bin/bash -l

mkdir -p ${slicedir}

##############################
# Make slices
##############################

echo "In makeSlices.sh : making the spectral slices for group ${GRP}"

slQsub=makeslices-GRP${GRP}.sbatch

if [ ${writeByNode} == true ]; then

    nsubxSlice=${nsubxCR}
    nsubySlice=${nsubyCR}
    width=${SLICERWIDTH}
    nppn=${SLICERNPPN}

else

    nsubxSlice=1
    nsubySlice=1
    width=1
    nppn=1

fi

# This is the new slicing job. If we're in here, the model
# cube exists in chunks created by the individual workers of
# createFITS. This job creates the individual slices by
# getting the appropriate slices of the chunks and stitching
# them together using makeAllModelSlices in Analysis.

cat > $slQsub <<EOF
#!/bin/bash -l
#SBATCH --time=12:00:00
#SBATCH --ntasks=${width}
#SBATCH --ntasks-per-node=${nppn}
#SBATCH --mail-user matthew.whiting@csiro.au
#SBATCH --job-name sliceCont
#SBATCH --mail-type=ALL
#SBATCH --no-requeue
#SBATCH --export=ASKAP_ROOT,AIPSPATH

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################

slicer=\${ASKAP_ROOT}/Code/Components/Analysis/simulations/current/apps/makeAllModelSlices.sh

slParset=${parsetdir}/makeslices-GRP${GRP}-\${SLURM_JOB_ID}.in
slLog=${logdir}/makeslices-GRP${GRP}-\${SLURM_JOB_ID}.log

cat >> \${slParset} <<EOFINNER
####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################
#
makeAllModelSlices.modelname = ${modelimage}
makeAllModelSlices.slicename = ${slicebase}
makeAllModelSlices.nsubx = ${nsubxSlice}
makeAllModelSlices.nsuby = ${nsubySlice}
makeAllModelSlices.nchanmodel = ${nchanSlicer}
makeAllModelSlices.firstchan = ${firstChanSlicer}
makeAllModelSlices.slicewidth = ${chanPerMSchunk}
makeAllModelSlices.npixslice = [${npixModel}, ${npixModel}]
makeAllModelSlices.nchanslice = ${chanPerMSchunk}
EOFINNER

aprun -n ${width} -N ${nppn} \${slicer} -c \${slParset} > \${slLog}

EOF


if [ $doSubmit == true ]; then

    export slID=`sbatch ${depend} ${slQsub} | awk '{print $4}'`
    echo Running cubeslice job with ID $slID

fi

