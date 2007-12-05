#!/bin/sh

echo "Running standard simulation and imaging for CONRAD"

echo "Extracting 10uJy model"

tar zxvf ${CONRAD_PROJECT_ROOT}/Code/Components/Synthesis/testdata/trunk/simulation/models/10uJy.model.small.tgz

export AIPSPATH=${CONRAD_PROJECT_ROOT}/Code/Components/Synthesis/testdata/trunk

echo "Running csimulator to create MeasurementSet for a single pointing"
time ${CONRAD_PROJECT_ROOT}/Code/Components/Synthesis/synthesis/trunk/bin/csimulator.sh -inputs stdtest.in

echo "Running cimager to form Clean image of single pointing"
time ${CONRAD_PROJECT_ROOT}/Code/Components/Synthesis/synthesis/trunk/bin/cimager.sh -inputs stdtest.in
