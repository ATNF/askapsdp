##############################################################################
# Running the image analysis
##############################################################################

if [ $CONTINUUMIMAGE == "" ]; then

        echo "Have not set \$CONTINUUMIMAGE, so not running analysis.sh"

else

#########################
# Making the sky model files for the comparison

    #skymodeldir=/scratch/astronomy116/whi550/DataChallenge/Simulation/SkyModel/current
    #cp ${skymodeldir}/duchamp-Results.txt skyModel-catalogue-results.txt
    #cp ${skymodeldir}/duchamp-Results.ann skyModel-catalogue-results.ann
    #perl -pi -e 's/RED/GREEN/g' skyModel-catalogue-results.ann
    
    #cp ${skymodeldir}/duchamp-fitResults.txt skyModel-catalogue.txt
    cp ${INPUT_SKYMODEL} skyModel-catalogue.txt
    awk '{if(NF==24) print $3,$4,$7,$15,$16,$9,$10,$11}' skyModel-catalogue.txt  > skyModel-catalogue-basic.txt
    awk '$2>-48. && $2<-42. && $1<191.875 && $1>183.25' skyModel-catalogue-basic.txt > skyModel-catalogue-basic-trim.txt
    awk '$3>0.05' skyModel-catalogue-basic-trim.txt > skyModel-catalogue-basic-trim-bright.txt
    sort -k3nr skyModel-catalogue-basic-trim.txt | head -20 > skyModel-catalogue-basic-trim-brightest.txt
    #cp ${skymodeldir}/duchamp-fitResults.ann skyModel-catalogue.ann
    #perl -pi -e 's/BLUE/YELLOW/g' skyModel-catalogue.ann
        
    #cp ${skymodeldir}/duchamp-SubimageLocations.ann skyModel-SubimageLocations.ann
    #perl -pi -e 's/YELLOW/WHITE/g' skyModel-SubimageLocations.ann

#########################


    qsubfile=analysis.qsub
    cat > ${qsubfile} <<EOF
#!/bin/bash -l
#PBS -W group_list=${QUEUEGROUP}
#PBS -l walltime=01:00:00
#PBS -l select=4:ncpus=4:mem=2GB:mpiprocs=4
##PBS -M first.last@csiro.au
#PBS -N analysis
##PBS -q debugq
#PBS -m bea
#PBS -j oe

cd \$PBS_O_WORKDIR

cduchamp=${ASKAP_ROOT}/Code/Components/Analysis/analysis/current/install/bin/cduchamp.sh
cimstat=${ASKAP_ROOT}/Code/Components/Analysis/analysis/current/install/bin/cimstat.sh
imagequal=${ASKAP_ROOT}/Code/Components/Analysis/analysis/current/install/bin/imageQualTest.sh
plotEval=${ASKAP_ROOT}/Code/Components/Analysis/evaluation/current/install/bin/plotEval.py
fluxEval=${ASKAP_ROOT}/Code/Components/Analysis/evaluation/current/install/bin/fluxEval.py

. ${ASKAP_ROOT}/Code/Components/Analysis/evaluation/current/init_package_env.sh

parset=analysis-\${PBS_JOBID}.in
cat > \$parset <<EOF_INNER
Cduchamp.image = ${CONTINUUMIMAGE}
Cduchamp.flagSubsection = true
Cduchamp.subsection = [601:2700,601:2700,*,*]
Cduchamp.snrCut = 6
Cduchamp.flagGrowth = true
Cduchamp.growthCut = 4
Cduchamp.doMedianSearch = true
Cduchamp.medianBoxWidth = 50
Cduchamp.doFit = true
Cduchamp.Fitter.fitTypes = [full]
Cduchamp.fitJustDetection = true
Cduchamp.Fitter.stopAfterFirstGoodFit = true
Cduchamp.nsubx = 5
Cduchamp.nsuby = 3
Cduchamp.minPix = 3
Cduchamp.minVoxels = 3
#
Cimstat.image = ${CONTINUUMIMAGE}
Cimstat.flagSubsection = true
Cimstat.subsection = [601:2700,601:2700,*,*]
#
imageQual.srcFile = duchamp-fitResults.txt
imageQual.refFile = skyModel-catalogue-basic-trim.txt
imageQual.image = ${CONTINUUMIMAGE}
imageQual.RA = 12:30:00
imageQual.Dec = -45:00:00
imageQual.epsilon = .3
imageQual.trimsize = 30
imageQual.convolveReference = false
EOF_INNER

statlog=log/cimstat-\${PBS_JOBID}.log
sflog=log/cduchamp-\${PBS_JOBID}.log
iqlog=log/imagequal-\${PBS_JOBID}.log
pelog=log/ploteval-\${PBS_JOBID}.log
felog=log/fluxeval-\${PBS_JOBID}.log

mpirun -np 1 \$cimstat -inputs \$parset > \$statlog
err=\$?
if [ \$err -ne 0 ]; then
    exit \$err
fi

mpirun \$cduchamp -inputs \$parset > \$sflog
err=\$?
if [ \$err -ne 0 ]; then
    exit \$err
fi

mpirun -np 1 \$imagequal -inputs \$parset > \$iqlog
err=\$?
if [ \$err -ne 0 ]; then
    exit \$err
fi

mpirun -np 1 \$plotEval > \$pelog
err=\$?
if [ \$err -ne 0 ]; then
    exit \$err
fi

mpirun -np 1 \$fluxEval > \$felog
exit \$?


EOF

    if [ "${DRYRUN}" == "false" ]; then

        # Submit the jobs
	if [ "${DEPENDS}" ]; then
            QSUB_ANALYSIS=`${QSUB_CMD} ${DEPENDS} ${qsubfile}`
	else
            QSUB_ANALYSIS=`${QSUB_CMD} ${qsubfile}`
            QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_ANALYSIS}"
	fi

    else

	echo "Analysis script (${CONTINUUMIMAGE}): Dry Run Only"

    fi



fi