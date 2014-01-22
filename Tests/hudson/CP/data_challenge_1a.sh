#!/bin/bash -l

# Save for later
EXEC_DATE=`date +%Y-%m-%d`

cd $WORKSPACE

#
# Bootstrap
#
unset ASKAP_ROOT
cd $WORKSPACE/trunk
nice python bootstrap.py -n
if [ $? -ne 0 ]; then
    echo "python bootstrap.py failed"
    exit 1
fi

#
# Setup environment
#
cd $WORKSPACE/trunk
source initaskap.sh
export AIPSPATH=$ASKAP_ROOT/Code/Base/accessors/current

#
# Build the rest of the packages needed to run the pipeline
#
nice rbuild -n Code/Components/Synthesis/synthesis/current
if [ $? -ne 0 ]; then
    echo "ERROR: Build of Code/Components/Synthesis/synthesis/current
failed"
    exit 1
fi
 
nice rbuild -n Code/Components/CP/pipelinetasks/current
if [ $? -ne 0 ]; then
    echo "ERROR: Build of Code/Components/CP/pipelinetasks/current failed"
    exit 1
fi

nice rbuild -n Code/Components/Analysis/analysis/current
if [ $? -ne 0 ]; then
    echo "ERROR: Build of Code/Components/Analysis/analysis/current failed"
    exit 1
fi

nice rbuild -n Code/Components/Analysis/evaluation/current
if [ $? -ne 0 ]; then
    echo "ERROR: Build of Code/Components/Analysis/evaluation/current
failed"
    exit 1
fi

# The "job" actually runs out of scratch, where the build is
# happening in the home directory
SCRATCH=/scratch/askap/askapops/dc1a-hudson-workspace

#
# Link to the dataset
#
cd $SCRATCH
if [ $? -ne 0 ]; then
    echo "ERROR: Scratch directory does not exist"
    exit 1
fi
rm -rf dc1a
mkdir -p $SCRATCH/dc1a
if [ $? -ne 0 ]; then
    echo "ERROR: Failed to create directory in scratch"
    exit 1
fi
cd $SCRATCH/dc1a
ln -s $SCRATCH/dc1a-input input

#
# Copy over the test scripts
#
cp -r ${ASKAP_ROOT}/Tests/data_challenge_1a/processing/* .
if [ $? -ne 0 ]; then
    echo "ERROR: Failed to copy over test scripts"
    exit 1
fi


#
# Execute the jobs
#
echo "#### Executing Pipeline ####"
./submit.sh
if [ $? -ne 0 ]; then
    echo "ERROR: submit.sh returned an error"
    exit 1
fi

#
# Wait for completion
#
cd $SCRATCH/dc1a/run_*
if [ $? -ne 0 ]; then
    echo "ERROR: run directory not found"
    exit 1
fi
if [ ! -f jobids.txt ]; then
    echo "ERROR: jobids.txt file not found"
    exit 1
fi

JOBIDS=`cat jobids.txt`

FINISHED=false
while [ "${FINISHED}" == "false" ]; do
    echo "#### Checking for job completion ####"

    # Assume finished unless we find a job that is not
    FINISHED=true
    
    for JOB in ${JOBIDS}; do
        qstat ${JOB}
        if [ $? -eq 0 ]; then
            FINISHED=false
        fi
    done

    if [ "${FINISHED}" == "false" ]; then
        echo "`date` - Waiting for job completion"
        sleep 600
    fi
done

# Add some files to the workspace to identify the build
echo ${BUILD_NUMBER} > BUILD_NUMBER.txt
echo ${SVN_REVISION} > SVN_REVISION.txt
echo ${EXEC_DATE} > EXEC_DATE.txt

#
# Create a tarball artifact
#
echo "####  Creating artifact tarball ####"
cd $SCRATCH/dc1a

tar -c -v --exclude MS -f artifact.tar run_*
if [ $? -ne 0 ]; then
    exit 1
fi
echo "#### Artifact generation complete ####"
