#!/bin/bash -l
export AIPSPATH=${ASKAP_ROOT}/Code/Components/Synthesis/testdata/trunk

HOSTNAME=`hostname -s`

echo "This is the ASKAPsoft stdtest. It will run for about 20-30 minutes."  | tee stdtest.$HOSTNAME.out

echo "Extracting 10uJy model" | tee -a stdtest.$HOSTNAME.out
tar zxvf ${ASKAP_ROOT}/Code/Components/Synthesis/testdata/trunk/simulation/models/10uJy.model.small.tgz | tee -a stdtest.$HOSTNAME.out

echo "Running csimulator to create MeasurementSet for a single pointing" | tee -a  stdtest.$HOSTNAME.out
${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/trunk/bin/csimulator.sh -inputs stdtest.in | tee -a stdtest.$HOSTNAME.out

echo "Running cimager to form Clean image of single pointing" | tee -a  stdtest.$HOSTNAME.out
${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/trunk/bin/cimager.sh -inputs stdtest.in | tee -a stdtest.$HOSTNAME.out
