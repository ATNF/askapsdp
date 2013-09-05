#!/usr/bin/env bash
##############################################################################
# Running the image analysis
##############################################################################

if [ $CONTINUUMIMAGE == "" ]; then
    echo "Have not set \$CONTINUUMIMAGE, so not running analysis.sh"
    return 1
fi

#########################
# Copying the sky model files for the cross-match
skymodel=skyModel-catalogue.txt
cp ${INPUT_SKYMODEL_TXT} ${skymodel}

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
#PBS -v ASKAP_ROOT,AIPSPATH

cd \$PBS_O_WORKDIR

selavy=${ASKAP_ROOT}/Code/Components/Analysis/analysis/current/apps/selavy.sh
cimstat=${ASKAP_ROOT}/Code/Components/Analysis/analysis/current/apps/cimstat.sh
crossmatch=${ASKAP_ROOT}/Code/Components/Analysis/analysis/current/apps/crossmatch.sh
plotEval=${ASKAP_ROOT}/Code/Components/Analysis/evaluation/current/install/bin/plotEval.py
fluxEval=${ASKAP_ROOT}/Code/Components/Analysis/evaluation/current/install/bin/fluxEval.py

. ${ASKAP_ROOT}/Code/Components/Analysis/evaluation/current/init_package_env.sh

parset=analysis-\${PBS_JOBID}.in
cat > \$parset <<EOF_INNER
Selavy.image = ${CONTINUUMIMAGE}
Selavy.flagSubsection = true
Selavy.subsection = ${ANALYSIS_SUBSECTION}
Selavy.snrCut = 6
Selavy.flagGrowth = true
Selavy.growthCut = 4
Selavy.VariableThreshold = true
Selavy.VariableThreshold.boxSize = 50
Selavy.Fitter.doFit = true
Selavy.Fitter.fitTypes = [full]
Selavy.Fitter.fitJustDetection = true
Selavy.Fitter.stopAfterFirstGoodFit = true
Selavy.nsubx = 5
Selavy.nsuby = 3
Selavy.minPix = 3
Selavy.minVoxels = 3
#
Cimstat.image = ${CONTINUUMIMAGE}
Cimstat.flagSubsection = true
Cimstat.subsection = ${ANALYSIS_SUBSECTION}
#
Crossmatch.source.filename     = selavy-fitResults.txt
Crossmatch.source.database     = Selavy
Crossmatch.source.trimsize     = 30
Crossmatch.reference.filename  = ${skymodel}
Crossmatch.reference.database  = Selavy
Crossmatch.reference.trimsize  = 30
Crossmatch.epsilon = 10arcsec
Crossmatch.matchfile = matches.txt
Crossmatch.missfile = misses.txt
#
Eval.refCatalogue = ${skymodel}
Eval.sourceCatalogue = selavy-fitResults.txt
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
sflog=log/selavy-\${PBS_JOBID}.log
cmlog=log/crossmatch-\${PBS_JOBID}.log
pelog=log/ploteval-\${PBS_JOBID}.log
felog=log/fluxeval-\${PBS_JOBID}.log

#mpirun -np 1 \$cimstat -c \$parset > \$statlog
casapy --nologger --log2term -c \$pystat > \$statlog
err=\$?
if [ \$err -ne 0 ]; then
    exit \$err
fi

mpirun \$selavy -c \$parset > \$sflog
err=\$?
if [ \$err -ne 0 ]; then
    exit \$err
fi

mpirun -np 1 \$crossmatch -c \$parset > \$cmlog
err=\$?
if [ \$err -ne 0 ]; then
    exit \$err
fi

evalparset=eval-parset-${PBS_JOBID}.in
grep "Eval" $parset > $evalparset

mpirun -np 1 \$plotEval -c \$evalparset > \$pelog
err=\$?
if [ \$err -ne 0 ]; then
    exit \$err
fi

mpirun -np 1 \$fluxEval -c \$evalparset > \$felog
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
if [ ! -e selavy-fitResults.txt ] || [ \\\`wc -l selavy-fitResults.txt | awk '{print \\\$1}'\\\` == 4 ]; then
    numcmpnt=0
else
    numcmpnt=\\\`wc -l selavy-fitResults.txt | awk '{print \\\$1-4}'\\\`
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
    xoffset=\\\`grep Offsets \$cmlog | tail -1 | awk '{print \\\$15}'\\\`
    xoffseterr=\\\`grep Offsets \$cmlog | tail -1 | awk '{print \\\$17}'\\\`
    yoffset=\\\`grep Offsets \$cmlog | tail -1 | awk '{print \\\$20}'\\\`
    yoffseterr=\\\`grep Offsets \$cmlog | tail -1 | awk '{print \\\$22}'\\\`
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
