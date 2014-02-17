#!/bin/bash -l


inputTrimFlag=${doTrim}

doTrim=true
POINTING=0
while [ $POINTING -lt 9 ]; do
    . ${scriptdir}/runImagingScienceField.sh
    depend="${depend}:${latestID}"
    POINTING=`expr $POINTING + 1`
done

doTrim=${inputTrimFlag}


linmosqsub=linmos_${tag}.qsub
cat > $linmosqsub <<EOF
#!/bin/bash -l
#PBS -l walltime=01:00:00
#PBS -l mppwidth=1
#PBS -l mppnppn=1
#PBS -N linmos
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

cd \$PBS_O_WORKDIR

linmos=${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/linmos.sh

suffix=uncor

parset=parsets/linmos_${suffix}.in
cat > $parset <<EOF_INNER
linmos.names = [image.i.dirty.science.${imtag}0..8.restored]
linmos.weights = [weights.i.dirty.science.${imtag}.0..8]
linmos.outname = image.i.dirty.science.${imtag}.linmos.restored
linmos.outweight = weights.i.dirty.science.${imtag}.linmos
linmos.weighttype = FromWeightImages
#linmos.weightstate  = Corrected
EOF_INNER
log=logs/linmos_${suffix}.log

aprun ${linmos} -c ${parset} > ${log}


EOF

qsub ${depend} ${linmosqsub}


