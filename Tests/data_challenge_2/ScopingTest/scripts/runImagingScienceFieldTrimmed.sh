#!/bin/bash -l


inputTrimFlag=${doTrim}
inputCalParams=${calParams}

doTrim=true
POINTING=0
imdepend=""
while [ $POINTING -lt 9 ]; do
    calParams=caldata-${POINTING}.dat
    . ${scriptdir}/runImagingScienceField.sh
    imdepend="${imdepend}:${latestID}"
    POINTING=`expr $POINTING + 1`
done

doTrim=${inputTrimFlag}
calParams=${inputCalParams}

. ${scriptdir}/getTags.sh

linmosqsub=linmos_${imtag}.qsub
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

parset=parsets/linmos_${imtag}.in
cat > \${parset} <<EOF_INNER
linmos.names = [image.i.dirty.science.${imtag}.0..8.restored]
linmos.weights = [weights.i.dirty.science.${imtag}.0..8]
linmos.outname = image.i.dirty.science.${imtag}.linmos.restored
linmos.outweight = weights.i.dirty.science.${imtag}.linmos
linmos.weighttype = FromWeightImages
#linmos.weightstate  = Corrected
EOF_INNER
log=logs/linmos_${imtag}.log

aprun \${linmos} -c \${parset} > \${log}


EOF

ID=`qsub -Wdepend=afterok${imdepend} ${linmosqsub}`
echo "Have submitted a linmos job for type ${imtag} with ID=${ID}, via 'qsub -Wdepend=afterok${imdepend} ${linmosqsub}'"



