#!/bin/bash -l


POINTING=0
while [ $POINTING -lt 9 ]; do
    . ${imScripts}/imageScienceFieldBeam.sh
    imdepend="${imdepend}:${latestID}"
    POINTING=`expr $POINTING + 1`
done

count=1
for type in image residual sensitivity; do

    if [ $doMFS == true ]; then
	mfsSuffix=".taylor.0"
	nterms="linmos.nterms     = 2"
    else
	mfsSuffix=""
	nterms="# no nterms parameter since not MFS"
    fi

    if [ $type == "image" ]; then
	imageInput="${imagebase}.BEAM0..8${mfsSuffix}.restored"
	imageOutput="${imagebase}.linmos${mfsSuffix}.restored"
    else
	sedstr="s/^image\./${type}\./g"
	imageInput=`echo ${imagebase} | sed -e ${sedstr}`.BEAM0..8${mfsSuffix}
	imageOutput=`echo ${imagebase} | sed -e ${sedstr}`.linmos${mfsSuffix}
    fi
    weightsInput=`echo ${imagebase} | sed -e 's/^image\./weights\./g'`.BEAM0..8${mfsSuffix}
    weightsOutput=`echo ${imagebase} | sed -e 's/^image\./weights\./g'`.linmos${mfsSuffix}

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
linmos.weighttype = FromWeightImages
linmos.psfref     = 4
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

