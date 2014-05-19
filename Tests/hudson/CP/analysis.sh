#!/bin/bash -l

#
# Bootstrap
#
unset ASKAP_ROOT
cd $WORKSPACE/trunk
nice python bootstrap.py -n

#
# Init askap environment
#
source initaskap.sh

#
# Build Analysis
#
RBUILD_OPTS="-n -p nompi=1"
nice rbuild ${RBUILD_OPTS} Code/Components/Analysis/analysisutilities/current
if [ $? -ne 0 ]; then
    echo "rbuild ${RBUILD_OPTS} Code/Components/Analysis/analysisutilities/current != 0"
    exit 1
fi

nice rbuild ${RBUILD_OPTS} Code/Components/Analysis/analysis/current
if [ $? -ne 0 ]; then
    echo "rbuild -${RBUILD_OPTS} Code/Components/Analysis/analysis/current != 0"
    exit 1
fi

nice rbuild ${RBUILD_OPTS} Code/Components/Analysis/simulations/current
if [ $? -ne 0 ]; then
    echo "rbuild ${RBUILD_OPTS} Code/Components/Analysis/simulations/current != 0"
    exit 1
fi

#
# Build testdata 
#
nice rbuild ${RBUILD_OPTS} Code/Base/accessors/current
if [ $? -ne 0 ]; then
    echo "rbuild ${RBUILD_OPTS} Code/Base/accessors/current != 0"
    exit 1
fi

#
# Run the unit tests
#
echo Running Unit Tests...
cd $ASKAP_ROOT/Code/Components/Analysis/analysisutilities/current
python build.py -q test
cd $ASKAP_ROOT/Code/Components/Analysis/analysis/current
python build.py -q test
cd $ASKAP_ROOT/Code/Components/Analysis/simulations/current
python build.py -q test
echo Completed Unit Tests

#
# Run a simple duchamp test
#
TESTPACKAGE=/DATA/DELPHINUS_1/whi550/Jenkins_Test_Packages/analysis-test.tgz
if [  ! -f $TESTPACKAGE ]; then
    echo "Test package $TESTPACKAGE not found"
    exit 1
fi

cd $WORKSPACE
nice tar zxvf $TESTPACKAGE
cd analysis-test
nice ./run.sh
if [ $? -ne 0 ]; then
    echo "Test returned != 0"
    exit 1
fi
