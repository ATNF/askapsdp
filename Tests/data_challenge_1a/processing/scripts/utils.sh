#!/usr/bin/env bash

# Args ($@) - Path/filename of the .qsub file file
#
# Environment Vairables:
# DEPENDS - List of dependencies to pass to qsub.
#           For example: afterok:1234.epic,afterok:2345.epic
#
# Returns:
# Echos the jobid and returns the status (zero is success, non-zero is failure)
# The output can be gathered like so:
# JOBID=`qsubmit acme.qsub`
# ERR=$?
function qsubmit()
{
    # Build the command and include dependencies if the DEPENDS environment
    # variable is set
    if [ ${DEPENDS} ]; then
        local QEXEC_CMD="${QSUB_CMD} -h -W depend=${DEPENDS} $@"
    else
        local QEXEC_CMD="${QSUB_CMD} -h $@"
    fi

    # Execute the command, or say what would be executed in the case of a dryrun
    if [ "${DRYRUN}" == "false" ]; then
        local OUTPUT=`${QEXEC_CMD}`
        local ERR=$?
    else
        echo "    DryRun - Would have executed: ${QEXEC_CMD}" >&2
        local OUTPUT="dryrun"
        local ERR=0
    fi

    echo ${OUTPUT}
    return ${ERR} 
}
