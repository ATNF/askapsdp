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

linmossbatch=linmos_${imtag}.sbatch
cat > $linmossbatch <<EOF
#!/bin/bash -l
#SBATCH --time=01:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name linmos${imtag}
#SBATCH --export=ASKAP_ROOT,AIPSPATH

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

ID=`sbatch --dependency=afterok${imdepend} ${linmossbatch} | awk '{print $4}'`
echo "Have submitted a linmos job for type ${imtag} with ID=${ID}, via 'sbatch --dependency=afterok${imdepend} ${linmossbatch}'"



