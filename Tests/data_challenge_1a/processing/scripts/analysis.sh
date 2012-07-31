#!/usr/bin/env bash
##############################################################################
# Running the image analysis
##############################################################################

if [ $CONTINUUMIMAGE == "" ]; then
    echo "Have not set \$CONTINUUMIMAGE, so not running analysis.sh"
    return 1
fi

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
#PBS -l select=2:ncpus=8:mem=4GB:mpiprocs=8
##PBS -M first.last@csiro.au
#PBS -N analysis
##PBS -q debugq
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT

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

pystat=getStats-\${PBS_JOBID}.py
cat > \${pystat} <<EOF_INNER
#!/bin/env python
## AUTOMATICALLY GENERATED!
# CASA script to obtain statistics for the image
ia.open('${CONTINUUMIMAGE}')
st=ia.statistics(region=rg.box(blc=[600,600,0,0],trc=[2699,2699,0,0]),robust=True)
madfmAsSigma=st['medabsdevmed'][0] / 0.6744888
print "Max = %5.3e = %3.1f sigma"%(st['max'][0],(st['max'][0]-st['median'][0])/madfmAsSigma)
print "Min = %5.3e = %3.1f sigma"%(st['min'][0],(st['min'][0]-st['median'][0])/madfmAsSigma)
print "Mean = %5.3e"%st['mean'][0]
print "Std.Dev = %5.3e"%st['sigma'][0]
print "Median = %5.3e"%st['median'][0]
print "MADFM = %5.3e"%st['medabsdevmed'][0]
print "MADFMasStdDev = %5.3e"%madfmAsSigma
ia.close()
EOF_INNER

statlog=log/cimstat-\${PBS_JOBID}.log
sflog=log/cduchamp-\${PBS_JOBID}.log
iqlog=log/imagequal-\${PBS_JOBID}.log
pelog=log/ploteval-\${PBS_JOBID}.log
felog=log/fluxeval-\${PBS_JOBID}.log

#mpirun -np 1 \$cimstat -inputs \$parset > \$statlog
casapy --nologger --log2term -c \$pystat > \$statlog
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
err=$?
if [ \$err -ne 0 ]; then
    exit $?
fi

##################
# Analysis summary
##################

summaryscript=analysis-summary-\${PBS_JOBID}.sh
cat > \$summaryscript <<EOF_INNER
#!/bin/bash -l

cwd=\\\`pwd\\\`
name=\\\`grep "Reading data" \$sflog | grep "(1, " | awk '{print \\\$12}'\\\`
max=\\\`grep Max \$statlog | awk '{print \\\$3}'\\\`
min=\\\`grep Min \$statlog | awk '{print \\\$3}'\\\`
median=\\\`grep Median \$statlog | awk '{print \\\$3}'\\\`
mean=\\\`grep Mean \$statlog | awk '{print \\\$3}'\\\`
madfm=\\\`grep "MADFM =" \$statlog | awk '{print \\\$3}'\\\`
madfmAsStddev=\\\`grep MADFMas \$statlog | awk '{print \\\$3}'\\\`
stddev=\\\`grep Std.Dev \$statlog | awk '{print \\\$3}'\\\`
numsrc=\\\`grep Found \$sflog | grep sources | awk '{print \\\$10}'\\\`
if [ ! -e duchamp-fitResults.txt ] || [ \\\`wc -l duchamp-fitResults.txt | awk '{print \\\$1}'\\\` == 0 ]; then
    numcmpnt=0
else
    numcmpnt=\\\`wc -l duchamp-fitResults.txt | awk '{print \\\$1-2}'\\\`
fi
if [ ! -e misses.txt ]; then
    nummiss=0
else
    nummiss=\\\`awk '\\\$1=="S"' misses.txt | wc -l\\\`
fi
if [ ! -e matches.txt ] || [ ! -e misses.txt ]; then
    xoffset=0
    xoffseterr=0
    yoffset=0
    yoffseterr=0
else
    xoffset=\\\`grep Offsets \$iqlog | awk '{print \\\$15}'\\\`
    xoffseterr=\\\`grep Offsets \$iqlog | awk '{print \\\$17}'\\\`
    yoffset=\\\`grep Offsets \$iqlog | awk '{print \\\$20}'\\\`
    yoffseterr=\\\`grep Offsets \$iqlog | awk '{print \\\$22}'\\\`
fi
imagerVersion=\\\`grep synthesis==current log/cimager-cont-clean-${QSUB_CONTCLEAN}.log | grep "(0, " | awk '{print \\\$12}'\\\`
analysisVersion=\\\`grep analysis==current \$sflog | grep "(0, " | awk '{print \\\$12}'\\\`

summary="analysis_summary.txt"
cat > \\\${summary} <<EOF_INNER2
#cwd,name,imagerVersion,analysisVersion,max,min,median,mean,madfm,stddev,madfmASstddev,numsrc,numcmpnt,nummiss,xoffset,xoffseterr,yoffset,yoffseterr
\\\$cwd,\\\$name,\\\$imagerVersion,\\\$analysisVersion,\\\$max,\\\$min,\\\$median,\\\$mean,\\\$madfm,\\\$stddev,\\\$madfmAsStddev,\\\$numsrc,\\\$numcmpnt,\\\$nummiss,\\\$xoffset,\\\$xoffseterr,\\\$yoffset,\\\$yoffseterr
EOF_INNER2
EOF_INNER

. \$summaryscript
EOF

# NOTE: Dependencies are passed in via the DEPENDS environment variable
echo "Analysis script (${CONTINUUMIMAGE}): Submitting"

# Submit the jobs
QSUB_ANALYSIS=`qsubmit ${qsubfile}`
if [ ! "${DEPENDS}" ]; then
    QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_ANALYSIS}"
fi
GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${QSUB_ANALYSIS}"
