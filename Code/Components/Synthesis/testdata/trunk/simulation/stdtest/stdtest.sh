#!/bin/bash -l

HOSTNAME=`hostname -s`

echo "Extracting 10uJy model" > stdtest.$HOSTNAME.out

tar zxvf ${ASKAP_ROOT}/Code/Components/Synthesis/testdata/trunk/simulation/models/10uJy.model.small.tgz >> stdtest.$HOSTNAME.out

export AIPSPATH=${ASKAP_ROOT}/Code/Components/Synthesis/testdata/trunk

echo "Running csimulator to create MeasurementSet for a single pointing" >> stdtest.$HOSTNAME.out
${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/trunk/bin/csimulator.sh -inputs stdtest.in >> stdtest.$HOSTNAME.out

echo "Running cimager to form Clean image of single pointing" >> stdtest.$HOSTNAME.out
${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/trunk/bin/cimager.sh -inputs stdtest.in >> stdtest.$HOSTNAME.out
