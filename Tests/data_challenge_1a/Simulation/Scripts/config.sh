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
posType=deg
PAunits=rad
WCSsources=true
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

diameter=12m

##############################
# Load the detailed parameters
##############################

. ${scriptdir}/configSmallBETA.sh
#. ${scriptdir}/configFullBETA.sh
#. ${scriptdir}/configASKAP12.sh
#. ${scriptdir}/configASKAP12_dec30.sh
#. ${scriptdir}/configASKAP12_dec04.sh
#
#. ${scriptdir}/configGASKAP.sh
#. ${scriptdir}/configM31.sh
#. ${scriptdir}/configBETAtestfield.sh

#. ${scriptdir}/config_fullASKAP_coarseContinuum.sh
#. ${scriptdir}/config_SKA1Survey.sh

##############################
# Cleaning up and verifying
##############################

basefreq=`echo $nchan $rchan $rfreq $chanw | awk '{printf "%8.6e",$3 + $4*($2+$1/2)}'`

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

msSlice=${msdir}/${msbase}_slice
msStage1=${msdir}/${msbase}_stage1
finalMS=${msdir}/${msbase}.ms

checkNumChan=`echo $chanPerMSchunk $NGROUPS_CSIM $NWORKERS_CSIM | awk '{print $1*$2*$3}'`
if [ $checkNumChan != $nchan ] && [ $doFlatSpectrum != "true" ]; then
    echo "ERROR - nchan = ${nchan},  but chanPerMSchunk = ${chanPerMSchunk}, NGROUPS_CSIM = ${NGROUPS_CSIM} and NWORKERS_CSIM = ${NWORKERS_CSIM} (product = ${checkNumChan})"
    echo Not running script.
    doSubmit=false
fi

