#!/bin/bash -l

##############################
# TOP LEVEL
##############################

if [ ${ASKAP_ROOT} == "" ]; then
    echo "Your \$ASKAP_ROOT variable is not set. Not running script!"
    exit 1
fi

export AIPSPATH=${ASKAP_ROOT}/Code/Base/accessors/current

crdir=${BASEDIR}/InputModels
visdir=${BASEDIR}/Visibilities
smdir=${BASEDIR}/SkyModel
catdir=${BASEDIR}/InputCatalogue

depend=""
now=`date +%F-%H%M`

queueName=routequeue

##############################
# FUNDADMENTAL
##############################

sourcelist=master_possum_catalogue_trim10x10deg.dat

doSmallBETA=true

#freqChanZeroMHz=1421
freqChanZeroMHz=1050

baseimage="DCmodel"
databaseCR=POSSUM
#databaseCR=POSSUMHI
if [ $databaseCR == "POSSUM" ]; then
    listtypeCR=continuum
    baseimage="${baseimage}_cont"
elif [ $databaseCR == "POSSUMHI" ]; then
    listtypeCR=spectralline
    baseimage="${baseimage}_HI"
fi

useGaussianComponents=true
msbase=DCvis

##############################
# Load the detailed parameters
##############################

#. ${scriptdir}/configSmallBETA.sh
#. ${scriptdir}/configFullBETA.sh
. ${scriptdir}/configASKAP12.sh


##############################
# Cleaning up and verifying
##############################

basefreq=`echo $nchan $rchan $rfreq $chanw | awk '{printf "%8.6e",$3 + $4*($2+$1/2)}'`

if [ `echo $chanPerMSchunk  $numMSchunks | awk '{print $1*$2}'` != $nchan ]; then
    echo ERROR - nchan = $nchan, but chanPerMShunk = $chanPerMSchunk and numMSchunks = $numMSchunks
    echo Not running script.
    doSubmit=false
fi

calibparset=rndgains.in
if [ $doCorrupt == true ]; then
    if [ $doAntennaBased == true ]; then
	antbasedname="antbased"
	randomgainsArgs="-a 6 -p ${npol}"
    else
	antbasedname="feedbased"
	randomgainsArgs="-f ${nfeeds} -a 6 -p ${npol}"
    fi
    corruptName="corrupt_${antbasedname}"
else
    corruptName="noCorrupt"
fi

msbase="${msbase}_${freqChanZeroMHz}_${polName}_${nfeeds}feeds_${noiseName}_${corruptName}"

msChunk=${msdir}/${msbase}_chunk
msStage1=${msdir}/${msbase}_stage1
finalMS=${msdir}/${msbase}.ms

numStage1jobs=`echo $numMSchunks $msPerStage1job | awk '{print int($1/$2)}'`
if [ `echo $msPerStage1job $numStage1jobs | awk '{print $1*$2}'` != $numMSchunks ]; then
    echo ERROR - numMSchunks = $numMSchunks and msPerStage1job = $msPerStage1job but numStage1jobs = $numStage1jobs
    echo Not running script.
    doSubmit=false
fi

