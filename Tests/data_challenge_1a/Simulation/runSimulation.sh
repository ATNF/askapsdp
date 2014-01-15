#!/bin/bash

# If ASKAP_ROOT is not set in your environment, add the path here and uncomment
#ASKAP_ROOT=<path to ASKAPsoft>

scriptdir=${ASKAP_ROOT}/Tests/data_challenge_1a/Simulation/Scripts

BASEDIR=`pwd`

if [ $# -ge 1 ]; then
    BASEDIR=$1
fi

CWD=`pwd`
cd ${BASEDIR}

config=${scriptdir}/config.sh

#doSubmit=true
doSubmit=false

unset QSUB_JOBLIST
unset depend

if [ ! -e numRuns ]; then
    echo 0 > numRuns
fi
RUN_NUM=`cat numRuns | awk '{print $1+1}'`
cat > numRuns <<EOF
$RUN_NUM
EOF
echo This will be run${RUN_NUM} of the simulation pipeline

doModelCreation=true
doVisibilities=true
doSkyModel=true

. ${config}

mkdir -p ${crdir}
mkdir -p ${visdir}
mkdir -p ${smdir}

if [ $doModelCreation == true ]; then
    
    echo "Running the create model script"
    . ${scriptdir}/Simulation-CreateModel.sh
    
fi

if [ $doVisibilities == true ]; then
    
    echo "Running the make visibilities script"
    . ${scriptdir}/Simulation-MakeVisibilities.sh
    
fi

if [ $doSkyModel == true ]; then
    
    echo "Running the make skymodel script"
    . ${scriptdir}/Simulation-MakeSkyModel.sh
    
fi

if [ $doSubmit == true ] && [ $QSUB_JOBLIST ]; then
    
    qrls $QSUB_JOBLIST
    
fi

cd $CWD



