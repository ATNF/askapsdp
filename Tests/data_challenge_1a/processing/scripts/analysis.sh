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

sbatchfile=analysis.sbatch
cat > ${sbatchfile} <<EOF
#!/bin/bash -l
#SBATCH --time=01:00:00
#SBATCH --ntasks=19
#SBATCH --ntasks-per-node=19
##SBATCH --mail-user first.last@csiro.au
#SBATCH --job-name analysis
#SBATCH --mail-type=ALL
#SBATCH --export=ASKAP_ROOT,AIPSPATH

selavy=${ASKAP_ROOT}/Code/Components/Analysis/analysis/current/apps/selavy.sh
cimstat=${ASKAP_ROOT}/Code/Components/Analysis/analysis/current/apps/cimstat.sh
crossmatch=${ASKAP_ROOT}/Code/Components/Analysis/analysis/current/apps/crossmatch.sh
plotEval=${ASKAP_ROOT}/Code/Components/Analysis/evaluation/current/install/bin/plotEval.py
fluxEval=${ASKAP_ROOT}/Code/Components/Analysis/evaluation/current/install/bin/fluxEval.py
imageEval=${ASKAP_ROOT}/Code/Components/Analysis/evaluation/current/install/bin/imageEval.py
finderEval=${ASKAP_ROOT}/Code/Components/Analysis/evaluation/current/install/bin/finderEval.py

. ${ASKAP_ROOT}/Code/Components/Analysis/evaluation/current/init_package_env.sh
. ${ASKAP_ROOT}/3rdParty/casacore/casacore-1.6.0a/init_package_env.sh

parset=analysis-\${SLURM_JOB_ID}.in
cat > \$parset <<EOF_INNER
Selavy.image = ${CONTINUUMIMAGE}
Selavy.flagSubsection = true
Selavy.subsection = ${ANALYSIS_SUBSECTION}
Selavy.snrCut = 6
Selavy.flagGrowth = true
Selavy.growthCut = 4
Selavy.VariableThreshold = true
Selavy.VariableThreshold.boxSize = 50
Selavy.VariableThreshold.ThresholdImageName=${THRESHIMAGE}
Selavy.VariableThreshold.NoiseImageName=${NOISEIMAGE}
Selavy.VariableThreshold.AverageImageName=${AVERAGEIMAGE}
Selavy.VariableThreshold.SNRimageName=${SNRIMAGE}
Selavy.Fitter.doFit = true
Selavy.Fitter.fitTypes = [full]
Selavy.Fitter.fitJustDetection = true
Selavy.Fitter.stopAfterFirstGoodFit = true
Selavy.nsubx = 6
Selavy.nsuby = 3
Selavy.minPix = 3
Selavy.minVoxels = 3
#
Cimstat.image = ${CONTINUUMIMAGE}
Cimstat.flagSubsection = true
Cimstat.subsection = ${ANALYSIS_SUBSECTION}
Cimstat.stats = ["Mean","Stddev","Median","MADFM","MADFMasStdDev"]
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
Eval.refCatalogue    = ${skymodel}
Eval.sourceCatalogue = selavy-fitResults.txt
Eval.thresholdImage  = ${THRESHIMAGE}.fits
Eval.noiseImage      = ${NOISEIMAGE}.fits
Eval.snrImage        = ${SNRIMAGE}.fits
Eval.image           = ${CONTINUUMIMAGE}.fits
Eval.sourceSelection = threshold
EOF_INNER

statlog=log/cimstat-\${SLURM_JOB_ID}.log
sflog=log/selavy-\${SLURM_JOB_ID}.log
cmlog=log/crossmatch-\${SLURM_JOB_ID}.log
pelog=log/ploteval-\${SLURM_JOB_ID}.log
felog=log/fluxeval-\${SLURM_JOB_ID}.log
ielog=log/imageval-\${SLURM_JOB_ID}.log
filog=log/imageval-\${SLURM_JOB_ID}.log

aprun -n 1 \$cimstat -c \$parset > \$statlog
err=\$?
if [ \$err -ne 0 ]; then
    exit \$err
fi
imnoise=\`grep MADFMas \$statlog | awk '{print \$10}'\`
cat >> \$parset <<EOF_INNER
Eval.imageNoise      = \${imnoise}
EOF_INNER

aprun -B \$selavy -c \$parset > \$sflog
err=\$?
if [ \$err -ne 0 ]; then
    exit \$err
fi

aprun -n 1 \$crossmatch -c \$parset > \$cmlog
err=\$?
if [ \$err -ne 0 ]; then
    exit \$err
fi

# Convert threshold/noise/snr maps to FITS
rm -f ${CONTINUUMIMAGE}.fits ${THRESHIMAGE}.fits ${NOISEIMAGE}.fits ${SNRIMAGE}.fits
aprun -n 1 image2fits in=${CONTINUUMIMAGE} out=${CONTINUUMIMAGE}.fits
aprun -n 1 image2fits in=${THRESHIMAGE} out=${THRESHIMAGE}.fits
aprun -n 1 image2fits in=${NOISEIMAGE} out=${NOISEIMAGE}.fits
aprun -n 1 image2fits in=${SNRIMAGE} out=${SNRIMAGE}.fits

evalparset=eval-parset-\${SLURM_JOB_ID}.in
grep "Eval" \$parset > \$evalparset

aprun -n 1 \$plotEval -c \$evalparset > \$pelog
err=\$?
if [ \$err -ne 0 ]; then
    exit \$err
fi

aprun -n 1 \$fluxEval -c \$evalparset > \$felog
err=$?
if [ \$err -ne 0 ]; then
    exit $?
fi

aprun -n 1 \$imageEval -c \$evalparset > \$ielog
err=$?
if [ \$err -ne 0 ]; then
    exit $?
fi

aprun -n 1 \$finderEval -c \$evalparset > \$filog
err=$?
if [ \$err -ne 0 ]; then
    exit $?
fi

##################
# Analysis summary
##################

summaryscript=analysis-summary-\${SLURM_JOB_ID}.sh
cat > \$summaryscript <<EOF_INNER
#!/bin/bash -l

cwd=\\\`pwd\\\`
name=\\\`grep "Reading data" \$sflog | grep "(1, " | awk '{print \\\$12}'\\\`
#max=\\\`grep Max \$statlog | awk '{print \\\$3}'\\\`
#min=\\\`grep Min \$statlog | awk '{print \\\$3}'\\\`
median=\\\`grep Median \$statlog | awk '{print \\\$10}'\\\`
mean=\\\`grep Mean \$statlog | awk '{print \\\$10}'\\\`
madfm=\\\`grep "MADFM =" \$statlog | awk '{print \\\$10}'\\\`
madfmAsStddev=\\\`grep MADFMas \$statlog | awk '{print \\\$10}'\\\`
stddev=\\\`grep Std.Dev \$statlog | awk '{print \\\$10}'\\\`
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
imagerVersion=\\\`grep synthesis==current log/cimager-cont-clean-${SBATCH_CONTCLEAN}.log | grep "(0, " | awk '{print \\\$12}'\\\`
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
SBATCH_ANALYSIS=`qsubmit ${sbatchfile}`
if [ ! "${DEPENDS}" ]; then
    SBATCH_NODEPS="${SBATCH_NODEPS} ${SBATCH_ANALYSIS}"
fi
GLOBAL_ALL_JOBS="${GLOBAL_ALL_JOBS} ${SBATCH_ANALYSIS}"
