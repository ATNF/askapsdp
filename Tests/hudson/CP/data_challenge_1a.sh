#!/bin/bash -l

# Save for later
EXEC_DATE=`date +%Y-%m-%d`

cd $WORKSPACE

#
# Bootstrap
#
cat > bootstrap.qsub << EOF
#!/bin/bash
#PBS -W group_list=astronomy554
#PBS -l select=1:ncpus=12:mem=12GB:mpiprocs=1
#PBS -l walltime=02:00:00
#PBS -N bootstrap
#PBS -m a
#PBS -j oe
#PBS -V

unset ASKAP_ROOT
cd $WORKSPACE/trunk
python bootstrap.py
if [ $? -ne 0 ]; then
    echo "python bootstrap.py failed"
    exit 1
fi
EOF

echo "#### Executing Bootstrap ####"
qsub -W block=true bootstrap.qsub
if [ $? -ne 0 ]; then
    echo "ERROR: bootstrap.qsub failed"
    exit 1
fi

#
# Setup environment
#
cd $WORKSPACE/trunk
source initaskap.sh
export AIPSPATH=$ASKAP_ROOT/Code/Base/accessors/current

#
# Execute matplotlib build on login node
# It depends on qt, which depends on packages
# not installed on the compute nodes
#
#cd $WORKSPACE/trunk/3rdParty/matplotlib
#nice rbuild -n -p mpi=1

cd $WORKSPACE
#
# Build the rest of the packages needed to run the pipeline
#
cat > build.qsub << EOF
#!/bin/bash
#PBS -W group_list=astronomy554
#PBS -l select=1:ncpus=12:mem=12GB:mpiprocs=1
#PBS -l walltime=04:00:00
#PBS -N build
#PBS -m a
#PBS -j oe
#PBS -V

#
# Setup environment
#
cd $WORKSPACE/trunk
source initaskap.sh
export AIPSPATH=$ASKAP_ROOT/Code/Base/accessors/current

rbuild -n -p mpi=1 Code/Components/Synthesis/synthesis/current
if [ $? -ne 0 ]; then
    echo "ERROR: Build of Code/Components/Synthesis/synthesis/current
failed"
    exit 1
fi
 
rbuild -n -p mpi=1 Code/Components/CP/pipelinetasks/current
if [ $? -ne 0 ]; then
    echo "ERROR: Build of Code/Components/CP/pipelinetasks/current failed"
    exit 1
fi

rbuild -n -p mpi=1 Code/Components/Analysis/analysis/current
if [ $? -ne 0 ]; then
    echo "ERROR: Build of Code/Components/Analysis/analysis/current failed"
    exit 1
fi

rbuild -n -p mpi=1 Code/Components/Analysis/evaluation/current
if [ $? -ne 0 ]; then
    echo "ERROR: Build of Code/Components/Analysis/evaluation/current
failed"
    exit 1
fi
EOF

echo "#### Executing Build ####"
qsub -W block=true build.qsub
if [ $? -ne 0 ]; then
    echo "ERROR: build.qsub failed"
    exit 1
fi


#
# Link to the dataset
#
mkdir -p $WORKSPACE/dc1a
cd $WORKSPACE/dc1a
ln -s /scratch/astronomy554/askapops/dc1a-input input

#
# Copy over the test scripts
#
cd $WORKSPACE/dc1a
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
cd $WORKSPACE/dc1a/run_*
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
cat > mkartifact.qsub << EOF
#!/bin/bash
#PBS -q copyq
#PBS -W group_list=astronomy554
#PBS -l select=1:ncpus=1:mem=2GB:mpiprocs=1
#PBS -l walltime=24:00:00
#PBS -N mkartifact
#PBS -m a
#PBS -j oe
#PBS -V

cd $WORKSPACE/dc1a

tar -c -v --exclude MS -f artifact.tar run_*
if [ $? -ne 0 ]; then
    exit 1
fi
EOF

echo "####  Creating artifact tarball ####"

qsub -W block=true mkartifact.qsub
if [ $? -ne 0 ]; then
    echo "ERROR: mkartifact.qsub failed"
    exit 1
fi
echo "#### Artifact generation complete ####"
