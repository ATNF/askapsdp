#!/bin/bash -l

mkdir -p ${slicedir}

##############################
# Make slices
##############################

if [ $doSlice == true ]; then 

    slQsub=${visdir}/${WORKDIR}/makeslices.qsub

    if [ ${writeByNode} == true ]; then

	nsubxSlice=${nsubxCR}
	nsubySlice=${nsubyCR}

    else

	nsubxSlice=1
	nsubySlice=1

    fi

	# This is the new slicing job. If we're in here, the model
	# cube exists in chunks created by the individual workers of
	# createFITS. This job creates the individual slices by
	# getting the appropriate slices of the chunks and stitching
	# them together using makeAllModelSlices in Analysis.

    cat > $slQsub <<EOF
#!/bin/bash -l
#PBS -l walltime=12:00:00
#PBS -l mppwidth=${SLICERWIDTH}
#PBS -l mppnppn=${SLICERNPPN}
#PBS -M matthew.whiting@csiro.au
#PBS -N sliceCont
#PBS -m bea
#PBS -j oe
#PBS -r n

####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################

cd \${PBS_O_WORKDIR}
export ASKAP_ROOT=${ASKAP_ROOT}
export AIPSPATH=\${ASKAP_ROOT}/Code/Base/accessors/current
slicer=\${ASKAP_ROOT}/Code/Components/Analysis/simulations/current/apps/makeAllModelSlices.sh

slParset=${parsetdirVis}/makeslices-\${PBS_JOBID}.in
slLog=${logdirVis}/makeslices-\${PBS_JOBID}.log

cat >> \${slParset} <<EOFINNER
####################
# AUTOMATICALLY GENERATED - DO NOT EDIT
####################
#
makeAllModelSlices.modelname = ${modelimage}
makeAllModelSlices.slicename = ${slicebase}
makeAllModelSlices.nsubx = ${nsubxCR}
makeAllModelSlices.nsuby = ${nsubyCR}
makeAllModelSlices.nchanmodel = ${nchanSlicer}
makeAllModelSlices.firstchan = ${firstChanSlicer}
makeAllModelSlices.slicewidth = ${chanPerMSchunk}
makeAllModelSlices.npixslice = [${npix}, ${npix}]
makeAllModelSlices.nchanslice = ${chanPerMSchunk}
EOFINNER

aprun -n ${SLICERWIDTH} -N ${SLICERNPPN} \${slicer} -c \${slParset} > \${slLog}

EOF

##     else
## 
## 	# This is the old slicing job. It assumes we have a complete
## 	# monolithic cube and uses cubeslice in Synthesis to carve off
## 	# slices.
## 
## 	cat > $slQsub <<EOF
## #!/bin/bash -l
## #PBS -l walltime=12:00:00
## #PBS -l mppwidth=1
## #PBS -l mppnppn=1
## #PBS -M matthew.whiting@csiro.au
## #PBS -N sliceCont
## #PBS -m bea
## #PBS -j oe
## #PBS -r n
## 
## ####################
## # AUTOMATICALLY GENERATED - DO NOT EDIT
## ####################
## 
## cd \$PBS_O_WORKDIR
## 
## export ASKAP_ROOT=${ASKAP_ROOT}
## export AIPSPATH=\${ASKAP_ROOT}/Code/Base/accessors/current
## cubeslice=\${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/cubeslice.sh
## 
## slLog=${logdirVis}/cubeslice-continuum-\${PBS_JOBID}.log
## 
## # make the models for each of the workers that hold the right number of channels
## echo Extracting chunks from cube \`date\` >> \$slLog
## CHUNK=0
## chanPerMS=${chanPerMSchunk}
## NCHUNKS=${NWORKERS_CSIM}
## startChan=0
## while [ \$CHUNK -lt \$NCHUNKS ]; do
##     chanPerMS=\`echo \$CHUNK ${nchan} \$chanPerMS | awk '{if(\$3>\$2-\$1*\$3) print \$2-\$1*\$3; else print \$3}'\`
##     chunkcube=${slicebase}\${CHUNK}
##     SECTION=\`echo \$startChan \$chanPerMS  | awk '{printf "[*,*,*,%d:%d]",\$1,\$1+\$2-1}'\`
##     echo "\$CHUNK: Saving model image \$chunkcube with \$chanPerMS channels, using section \$SECTION" >> \$slLog
##     \$cubeslice -s \$SECTION $modelimage \$chunkcube
##     err=\$?
##     if [ \$err != 0 ]; then
##        exit \$err
##     fi
##     CHUNK=\`expr \$CHUNK + 1\`
##     startChan=\`expr \$startChan + \$chanPerMS\`
## done
## exit \$err
## 
## EOF
## 
##     fi

    if [ $doSubmit == true ]; then

	export slID=`qsub ${depend} ${slQsub}`
	echo Running cubeslice job with ID $slID

    fi

fi
