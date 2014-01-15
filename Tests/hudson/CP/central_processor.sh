#!/bin/bash -l

#
# Bootstrap
#
unset ASKAP_ROOT
cd $WORKSPACE/trunk
nice python2.6 bootstrap.py -n

#
# Init ASKAP environment
#
source initaskap.sh
export AIPSPATH=$ASKAP_ROOT/Code/Base/accessors/current

#
# Build CP
#
nice rbuild -n Code/Components/CP
if [ $? -ne 0 ]; then
    echo "rbuild -n Code/Components/CP != 0"
    exit 1
fi


#
# Run the unit tests
#
echo Running Unit Tests...
cd $WORKSPACE/trunk/Code/Components/CP
nice rbuild -n -q -t test
echo Completed Unit Tests

#
# Build testdata for the functional tests
#
cd $WORKSPACE/trunk
nice rbuild -n Code/Base/accessors/current
if [ $? -ne 0 ]; then
    echo "rbuild -n Code/Base/accessors/current != 0"
    exit 1
fi

#
# Run the functests tests for the imager
#
#cd $WORKSPACE/trunk/Code/Components/CP/imager/current/functests
#nice ./run.sh
#if [ $? -ne 0 ]; then
#    echo CP Imager Functional Testsuite returned != 0
#    exit 1
#fi

#
# Run the functests tests for the ingest pipeline
#
#cd $WORKSPACE/trunk/Code/Components/CP/ingest/current/functests
#./run.sh
#if [ $? -ne 0 ]; then
#    echo CP Ingest Pipeline Functional Testsuite returned != 0
#    exit 1
#fi

#
# Run the functests tests for the manager
#
#cd $WORKSPACE/trunk/Code/Components/CP/manager/current/functests
#nice ./run.sh
#if [ $? -ne 0 ]; then
#    echo CP Manager Functional Testsuite returned != 0
#    exit 1
#fi

#
# Run the functests tests for the correlator simulator
# NOTE: No dataset so can't run this yet
#
#cd $WORKSPACE/trunk/Code/Components/CP/correlatorsim/current/functests
#./run.sh
#if [ $? -ne 0 ]; then
#    echo CP Correlator Simulator Functional Testsuite returned != 0
#    exit 1
#fi

#
# Run the functests tests for the pipeline tasks package
#
cd $WORKSPACE/trunk/Code/Components/CP/pipelinetasks/current/functests
nice ./run.sh
if [ $? -ne 0 ]; then
    echo CP PipelineTasks Functional Testsuite returned != 0
    exit 1
fi
