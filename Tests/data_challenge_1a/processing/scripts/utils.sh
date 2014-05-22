#!/usr/bin/env bash

# Args ($@) - Path/filename of the .sbatch file file
#
# Environment Vairables:
# DEPENDS - List of dependencies to pass to qsub.
#           For example: afterok:1234.epic,afterok:2345.epic
#
# Returns:
# Echos the jobid and returns the status (zero is success, non-zero is failure)
# The output can be gathered like so:
# JOBID=`qsubmit acme.sbatch`
# ERR=$?
function qsubmit()
{
    # Build the command and include dependencies if the DEPENDS environment
    # variable is set
    if [ "${DEPENDS}" ]; then
        local QEXEC_CMD="${SBATCH_CMD} --dependency=afterok:${DEPENDS} $@"
    else
        # All jobs with no dependencies are queued with a user hold. This ensures
        # these jobs are in the queue (and have not executed and completed) so 
        # other jobs can be made dependant on them. That means, that jobs without
        # dependencies must be released with "qrls".
        local QEXEC_CMD="${SBATCH_CMD} --hold $@"
    fi

    # Execute the command, grabbing the ID number, or say what would be 
    # executed in the case of a dryrun
    if [ "${DRYRUN}" == "false" ]; then
        local OUTPUT=`${QEXEC_CMD} | awk '{print $4}'`
	echo "    Executed: ${QEXEC_CMD}" >&2
        local ERR=$?
    else
        echo "    DryRun - Would have executed: ${QEXEC_CMD}" >&2
        local OUTPUT="dryrun"
        local ERR=0
    fi

    echo ${OUTPUT}
    return ${ERR} 
}
