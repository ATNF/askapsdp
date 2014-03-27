#!/bin/bash -l


POINTING=0
while [ $POINTING -lt 9 ]; do
    . ${imScripts}/imageScienceFieldBeam.sh
    imdepend="${imdepend}:${latestID}"
    POINTING=`expr $POINTING + 1`
done

count=1
#for type in image residual sensitivity; do
for type in image residual; do

    sedstr="s/^image\./${type}\./g"
    basename=`echo ${imagebase} | sed -e ${sedstr}`

    if [ $doMFS == true ]; then
	mfsSuffix=".taylor.0"
	nterms="linmos.nterms     = 2"
    else
	mfsSuffix=""
	nterms="# no nterms parameter since not MFS"
    fi

    if [ $type == "image" ]; then
	imSuffix=".restored"
    else
	imSuffix=""
    fi
    imageInput="${basename}.BEAM0..8${mfsSuffix}${imSuffix}"
    imageOutput="${basename}.linmos${mfsSuffix}${imSuffix}"
    

    weightsInput=`echo ${imagebase} | sed -e 's/^image\./weights\./g'`.BEAM0..8${mfsSuffix}
    weightsOutput=`echo ${imagebase} | sed -e 's/^image\./weights\./g'`.linmos${mfsSuffix}

    if [ ${IMAGING_GRIDDER} == "AWProject" ]; then
	weightingPars="# Use the weight images directly:
linmos.weighttype = FromWeightImages
"
    else
	if [ ${model} == "SKADS" ]; then
	    weightingPars="# Use primary beam models at specific positions:
linmos.weighttype = FromPrimaryBeamModel
linmos.feeds.centre = [12h30m00.00, -45.00.00.00]
linmos.feeds.spacing = 1deg
linmos.feeds.${basename}.BEAM0${mfsSuffix}${imSuffix} = [-1.0, -1.0]
linmos.feeds.${basename}.BEAM1${mfsSuffix}${imSuffix} = [-1.0,  0.0]
linmos.feeds.${basename}.BEAM2${mfsSuffix}${imSuffix} = [-1.0,  1.0]
linmos.feeds.${basename}.BEAM3${mfsSuffix}${imSuffix} = [ 0.0, -1.0]
linmos.feeds.${basename}.BEAM4${mfsSuffix}${imSuffix} = [ 0.0,  0.0]
linmos.feeds.${basename}.BEAM5${mfsSuffix}${imSuffix} = [ 0.0,  1.0]
linmos.feeds.${basename}.BEAM6${mfsSuffix}${imSuffix} = [-1.0, -1.0]
linmos.feeds.${basename}.BEAM7${mfsSuffix}${imSuffix} = [ 0.0,  0.0]
linmos.feeds.${basename}.BEAM8${mfsSuffix}${imSuffix} = [ 1.0,  1.0]
"
	else
	    weightingPars="# Use primary beam models at specific positions:
linmos.weighttype = FromPrimaryBeamModel
linmos.feeds.centre = [12h30m00.00, -45.00.00.00]
linmos.feeds.spacing = 1deg
linmos.feeds.${basename}.BEAM0${mfsSuffix}${imSuffix} = [0,0]
linmos.feeds.${basename}.BEAM1${mfsSuffix}${imSuffix} = [-0.572425, 0.947258]
linmos.feeds.${basename}.BEAM2${mfsSuffix}${imSuffix} = [-1.14485, 1.89452]
linmos.feeds.${basename}.BEAM3${mfsSuffix}${imSuffix} = [0.572425, -0.947258]
linmos.feeds.${basename}.BEAM4${mfsSuffix}${imSuffix} = [-1.23347, -0.0987957]
linmos.feeds.${basename}.BEAM5${mfsSuffix}${imSuffix} = [-1.8059, 0.848462]
linmos.feeds.${basename}.BEAM6${mfsSuffix}${imSuffix} = [0.661046, 1.04605]
linmos.feeds.${basename}.BEAM7${mfsSuffix}${imSuffix} = [0.0886209, 1.99331]
linmos.feeds.${basename}.BEAM8${mfsSuffix}${imSuffix} = [1.23347, 0.0987957]
"
	fi
    fi

    linmosqsub=linmos_${type}.qsub
    cat > $linmosqsub <<EOF
#!/bin/bash -l
#PBS -l walltime=01:00:00
#PBS -l mppwidth=1
#PBS -l mppnppn=1
#PBS -N linmos${count}
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

cd \$PBS_O_WORKDIR

linmos=\${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/linmos.sh

parset=parsets/linmos_${type}_\${PBS_JOBID}.in
cat > \${parset} <<EOF_INNER
linmos.names      = [${imageInput}]
linmos.weights    = [${weightsInput}]
linmos.outname    = ${imageOutput}
linmos.outweight  = ${weightsOutput}
linmos.psfref     = 4
${weightingPars}
${nterms}
EOF_INNER
log=logs/linmos_${type}_\${PBS_JOBID}.log

aprun \${linmos} -c \${parset} > \${log}


EOF

    if [ $doSubmit == true ]; then 
	ID=`qsub ${imdepend} ${linmosqsub}`
	echo "Have submitted a linmos job for image type '${type}' with ID=${ID}, via 'qsub ${imdepend} ${linmosqsub}'"
    fi

    count=`expr $count + 1`

done

