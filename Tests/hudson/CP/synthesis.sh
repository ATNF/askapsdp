#!/bin/bash
unset ASKAP_ROOT
cd $WORKSPACE/trunk
nice /usr/bin/python2.6 bootstrap.py
if [ $? - ne 0 ]; then
    echo "Error: Bootstrapping failed"
    exit 1
fi

#
# Setup environment
#
source initaskap.sh
if [ $MODULESHOME ]; then
    module load openmpi
fi

#
# Build Synthesis
#
nice rbuild -n Code/Components/Synthesis/synthesis/current
if [ $? - ne 0 ]; then
    echo "Error: rbuild -n Code/Components/Synthesis/synthesis/current failed "
    exit 1
fi

#
# Build testdata for the unit tests
#
nice rbuild -n Code/Components/Synthesis/testdata/current
if [ $? -ne 0 ]; then
    echo "rbuild -n Code/Components/Synthesis/testdata/current failed"
    exit 1
fi

#
# Run the unit tests
#
cd $WORKSPACE/trunk/Code/Components/Synthesis/synthesis/current
scons test

cd $WORKSPACE/trunk/Code/Base/scimath/current
scons test

cd $WORKSPACE/trunk/Code/Base/accessors/current
scons test

#
# Phase 2
#
#
# Run the synthregression test
#
cd $WORKSPACE/trunk
source initaskap.sh
cd $WORKSPACE/trunk/Code/Components/Synthesis/testdata/current/simulation/synthregression
python synthregression.py
ERROR=$?
if [ $ERROR -ne 0 ]; then
    echo "synthregression.py returned errorcode $ERROR"
    exit 1
fi

#
# Phase 3
#
#
# Run the stdtest and verify results
#

# Output file, so we can search for errors/exceptions, etc..
OUTPUT_FILE=$WORKSPACE/stdtest.out

cd $WORKSPACE/trunk
source initaskap.sh

# Setting TESTDIR must happen after the initaskap.sh
TESTDIR=$ASKAP_ROOT/Code/Components/Synthesis/testdata/current

if [ $MODULESHOME ]; then
    module load openmpi
fi

# Run rbuild in the test directory
if [ -d $TESTDIR ]; then
    cd $ASKAP_ROOT
    rbuild -n $TESTDIR
else
    echo "Error: $TESTDIR does not exist"
    exit 1
fi

# Check for the existence of the test script
if [ ! -x $TESTDIR/simulation/stdtest/stdtest.sh ]; then
    echo "Error: Executable script stdtest.sh not found"
    exit 1
fi

# Run the stdtest and check return code
cd $TESTDIR/simulation/stdtest
BEGIN=`date "+%s"`
./stdtest.sh | tee -a $OUTPUT_FILE
END=`date "+%s"`
if [ $? -ne 0 ]; then
    echo "Error: stdtest.sh returned an error"
    exit 1
fi

# Check for instances of "Askap error"
grep -c "Askap error" $OUTPUT_FILE > /dev/null
if [ $? -ne 1 ]; then
    echo "Error: An error was report in the stdtest output file:"
    echo
    grep "Askap error" $OUTPUT_FILE
    echo
    exit 1
fi

# Check for instances of "Unexpected exception"
grep -c "Unexpected exception" $OUTPUT_FILE > /dev/null
if [ $? -ne 1 ]; then
    echo "Error: An exception was report in the stdtest output file:"
    echo
    grep "Unexpected exception" $OUTPUT_FILE
    echo
    exit 1
fi

# Ensure the measurement set was created
if [ ! -d $TESTDIR/simulation/stdtest/10uJy_stdtest.ms ]; then
    echo "Error: measurement set was not created"
    echo
    exit 1
fi

# Ensure the dirty image was created
if [ ! -d $TESTDIR/simulation/stdtest/image.i.10uJy_dirty_stdtest ]; then
    echo "Error: image.i.10uJy_dirty_stdtest was not created"
    echo
    exit 1
fi

# Ensure the psf was created
if [ ! -d $TESTDIR/simulation/stdtest/psf.i.10uJy_dirty_stdtest ]; then
    echo "Error: psf.i.10uJy_dirty_stdtest was not created"
    echo
    exit 1
fi

# Ensure image.i.10uJy_clean_stdtest.restored was created
if [ ! -d $TESTDIR/simulation/stdtest/image.i.10uJy_clean_stdtest.restored ]; then
    echo "Error: image.i.10uJy_clean_stdtest.restored was not created"
    echo
    exit 1
fi

# If it worked ok, just report the runtime
TIME=`expr $END - $BEGIN`
echo
echo "==================== ADDITIONAL INFORMATION ===================="
echo
if [ $TIME -lt 300 ]; then
    echo Runtime for stdtest: $TIME seconds
else
    TIME=`expr $TIME / 60`
    echo Runtime for stdtest: $TIME minutes
fi

