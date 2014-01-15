#!/bin/bash -l

#
# Phase 1
#
# Nice the processes running on the head node, since we don't really
# want then to interfere with jobs running if possible
NICE="nice -n 10"

module load openmpi

unset ASKAP_ROOT
cd $WORKSPACE/trunk
$NICE /usr/bin/python2.6 bootstrap.py -n
if [ $? -ne 0 ]; then
    echo "Error: Bootstrapping failed"
    exit 1
fi

#
# Phase 2
#
#
# Setup environment
#
cd $WORKSPACE/trunk
source /etc/profile.d/modules.sh
source initaskap.sh
if [ $MODULESHOME ]; then
    module load openmpi
else
    echo 'Error: $MODULESHOME is not defined'
    exit 1
fi

cd $ASKAP_ROOT

$NICE rbuild -n Code/Components/Synthesis/synthesis/current
if [ $? -ne 0 ]; then
   echo "rbuild Code/Components/Synthesis/synthesis/current return !=0"
   exit 1
fi

#
# Phase 3
#
# Obtain the test package
mkdir $WORKSPACE/cimager
$NICE cp /exported2/hudson-test-packages/2km1400Mhz/2km1400Mhz.qsub $WORKSPACE/cimager
if [ $? -ne 0 ]; then
   echo "copy of 2km1400Mhz.qsub to $WORKSPACE/cimager directory failed."
   exit 1
fi

$NICE cp -r /exported2/hudson-test-packages/2km1400Mhz/definitions $WORKSPACE/cimager
if [ $? -ne 0 ]; then
   echo "copy of definitions to $WORKSPACE/cimager directory failed."
   exit 1
fi


#
# Phase 4
#
# Run the job
cd $WORKSPACE/cimager
jobid=`qsub 2km1400Mhz.qsub`
if [ $? -ne 0 ]; then
  echo "Error: Elais batch job returned != 0"
  exit 1
fi

# Display the queue
echo "Torque queue:"
qstat -a
echo

# Wait for job completion
qstat $jobid 2>&1 > /dev/null
while [ $? -eq 0 ]; do
  echo "`date` - Waiting for job $jobid completion"
  sleep 300
  qstat $jobid 2>&1 > /dev/null
done

echo "`date` - Job completed, displaying log file and verifying outputs"

OUTPUT_FILE=output.out

echo "Below is the log file:"
cat $OUTPUT_FILE
echo
echo "Verifying output..."

# Check for instances of "Askap error"
grep -c "Askap error" $OUTPUT_FILE > /dev/null
if [ $? -ne 1 ]; then
    echo "Error: An error was reported in the output file $OUTPUT_FILE:"
    echo
    grep "Askap error" $OUTPUT_FILE
    echo
    exit 1
fi

# Check for instances of "Unexpected exception"
grep -c "Unexpected exception" $OUTPUT_FILE > /dev/null
if [ $? -ne 1 ]; then
    echo "Error: An exception was reported in the output file $OUTPUT_FILE:"
    echo
    grep "Unexpected exception" $OUTPUT_FILE
    echo
    exit 1
fi
