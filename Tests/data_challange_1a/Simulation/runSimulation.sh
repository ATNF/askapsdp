#!/bin/bash -l

export ASKAP_ROOT=/home/whi550/askapsoft

#BASEDIR=/scratch/astronomy116/whi550/DataChallenge/Simulation
BASEDIR=`pwd`
scriptdir=${BASEDIR}/Scripts
config=${scriptdir}/config.sh

doSubmit=true

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

doModelCreation=false
doVisibilities=false
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

