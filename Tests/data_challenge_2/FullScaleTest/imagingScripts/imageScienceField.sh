#!/bin/bash -l


POINTING=0
if [ "${calDepend}" == "" ]; then
    imdepend="-Wdepend=afterok"
else
    imdepend=${calDepend}
fi
while [ $POINTING -lt 9 ]; do
    . ${imScripts}/imageScienceFieldBeam.sh
    imdepend="${imdepend}:${latestID}"
    POINTING=`expr $POINTING + 1`
done

TAYLOR=0
while [ $TAYLOR -le 2 ]; do

    imageInput="${imagebase}.BEAM0..8.taylor.${TAYLOR}.restored"
    imageOutput="${imagebase}.linmos.taylor.${TAYLOR}.restored"
    weightsInput=`echo ${imagebase} | sed -e 's/^image\./weights\./g'`.BEAM0..8.taylor.${TAYLOR}
    weightsOutput=`echo ${imagebase} | sed -e 's/^image\./weights\./g'`.linmos.taylor.${TAYLOR}

    linmosqsub=linmos_Taylor${TAYLOR}.qsub
    cat > $linmosqsub <<EOF
#!/bin/bash -l
#PBS -l walltime=01:00:00
#PBS -l mppwidth=1
#PBS -l mppnppn=1
#PBS -N linmos${imtag}
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

cd \$PBS_O_WORKDIR

linmos=\${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/linmos.sh

parset=parsets/linmos_Taylor${TAYLOR}_\${PBS_JOBID}.in
cat > \${parset} <<EOF_INNER
linmos.names = [${imageInput}]
linmos.weights = [${weightsInput}]
linmos.outname = ${imageOutput}
linmos.outweight = ${weightsOutput}
linmos.weighttype = FromWeightImages
#linmos.weightstate  = Corrected
linmos.psfref = 4
EOF_INNER
log=logs/linmos_\${PBS_JOBID}.log

aprun \${linmos} -c \${parset} > \${log}


EOF

    if [ $doSubmit == true ]; then 
	ID=`qsub ${imdepend} ${linmosqsub}`
	echo "Have submitted a linmos job for type ${imtag} with ID=${ID}, via 'qsub ${imdepend} ${linmosqsub}'"
    fi

    TAYLOR=`expr $TAYLOR + 1`

done

