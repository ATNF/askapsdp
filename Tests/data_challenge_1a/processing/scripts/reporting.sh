#!/usr/bin/env bash
##############################################################################
# Reporting the results of the data challenge run
##############################################################################

qsubfile=reporting.qsub
cat > $qsubfile <<EOF
#!/bin/bash
#PBS -W group_list=${QUEUEGROUP}
#PBS -l walltime=00:30:00
#PBS -l select=1:ncpus=1:mem=1GB:mpiprocs=8
##PBS -M first.last@csiro.au
#PBS -N report
#PBS -m bea
#PBS -j oe

cd \$PBS_O_WORKDIR

# This is the overall SUCCESS/FAILURE indicator.
overall=0

# Initialise the individual exit codes to '-', indicating they are not
# done.
exitcodeSPL="-"
exitcodeCMODEL="-"
exitcodeCAL="-"
exitcodeCD="-"
exitcodeCC="-"
exitcodeCCD="-"
exitcodeCCC="-"
exitcodeCCDM="-"
exitcodeCCCM="-"
exitcodeSL1="-"
exitcodeSL2="-"
exitcodeSLM="-"
exitcodeAN="-"

logfileSPL="\`echo split.o${QSUB_MSSPLIT}.* | sed -e 's/\[\].epic//g'\`"
if [ ! -e \`echo split.o${QSUB_MSSPLIT}.0 | sed -e 's/\[\].epic//g'\` ]; then
    overall=1
else
    exitcodeSPL=\`grep -h Exit \${logfileSPL} | sort | uniq | awk '{print \$3}'\`
    if [ \$exitcodeSPL != 0 ]; then
        overall=1
    fi
fi

if [ ${DO_CALIBRATION} == true ]; then
    logfileCMODEL="\`echo cmodel.o${QSUB_CMODEL} | sed -e 's/.epic//g'\`"
    if [ ! -e ${logfileCMODEL} ]; then
        overall=1
    else
        exitcodeCMODEL=\`grep Exit \${logfileCMODEL} | awk '{print \$3}'\`
        if [ \$exitcodeCMODEL != 0 ]; then
    	overall=1
        fi
    fi
    logfileCAL="\`echo ccalibrator.o${QSUB_CAL} | sed -e 's/.epic//g'\`"
    if [ ! -e ${logfileCAL} ]; then
        overall=1
    else
        exitcodeCAL=\`grep Exit \${logfileCAL} | awk '{print \$3}'\`
        if [ \$exitcodeCAL != 0 ]; then
    	overall=1
        fi
    fi
fi

if [ ${DO_CONTINUUM_DIRTY} == true ]; then
    logfileCD="\`echo cont-dirty.o${QSUB_CONTDIRTY} | sed -e 's/.epic//g'\`"
    if [ ! -e ${logfileCD} ]; then
        overall=1
    else
        exitcodeCD=\`grep Exit \${logfileCD} | awk '{print \$3}'\`
        if [ \$exitcodeCD != 0 ]; then
    	overall=1
        fi
    fi
fi

if [ ${DO_CONTINUUM_CLEAN} == true ]; then
    logfileCC="\`echo cont-clean.o${QSUB_CONTCLEAN} | sed -e 's/.epic//g'\`"
    if [ ! -e ${logfileCC} ]; then
        overall=1
    else
        exitcodeCC=\`grep Exit \${logfileCC} | awk '{print \$3}'\`
        if [ \$exitcodeCC != 0 ]; then
    	overall=1
        fi
    fi
fi

if [ ${DO_ANALYSIS} == true ]; then
    logfileAN="\`echo analysis.o${QSUB_ANALYSIS} | sed -e 's/.epic//g'\`"
    if [ ! -e ${logfileAN} ]; then
        overall=1
    else
        exitcodeAN=\`grep Exit \${logfileAN} | awk '{print \$3}'\`
        if [ \$exitcodeAN != 0 ]; then
    	overall=1
        fi
    fi
fi

if [ ${DO_CONTINUUM_CUBE_DIRTY} == true ]; then
    logfileCCD="\`echo contcube-dirty.o${QSUB_CONTCUBEDIRTY}.* | sed -e 's/\[\].epic//g'\`"
    if [ ! -e \`echo contcube-dirty.o${QSUB_CONTCUBEDIRTY}.0 | sed -e 's/\[\].epic//g'\` ]; then
        overall=1
    else
        exitcodeCCD=\`grep -h Exit \${logfileCCD} | sort | uniq | awk '{print \$3}'\`
        if [ \$exitcodeCCD != 0 ]; then
    	overall=1
        fi
    fi
    logfileCCDM="makecube.o*"
    exitcodeCCDM=\`grep -h Exit \${logfileCCDM} | sort | uniq | awk '{print \$3}'\`
    if [ \$exitcodeCCDM != 0 ]; then
	overall=1
    fi
fi

if [ ${DO_CONTINUUM_CUBE_CLEAN} == true ]; then
    logfileCCC="\`echo contcube-clean.o${QSUB_CONTCUBECLEAN}.* | sed -e 's/\[\].epic//g'\`"
    if [ ! -e \`echo contcube-clean.o${QSUB_CONTCUBECLEAN}.0 | sed -e 's/\[\].epic//g'\` ]; then
        overall=1
    else
        exitcodeCCC=\`grep -h Exit \${logfileCCC} | sort | uniq | awk '{print \$3}'\`
        if [ \$exitcodeCCC != 0 ]; then
    	overall=1
        fi
    fi
    logfileCCCM="makecube.o*"
    exitcodeCCCM=\`grep -h Exit \${logfileCCCM} | sort | uniq | awk '{print \$3}'\`
    if [ \$exitcodeCCCM != 0 ]; then
	overall=1
    fi
fi

if [ ${DO_SPECTRAL_LINE} == true ]; then
    logfileSL1="\`echo sl-img.o${QSUB_SPECTRAL1}.* | sed -e 's/\[\].epic//g'\`"
    if [ ! -e \`echo sl-img.o${QSUB_SPECTRAL1}.0 | sed -e 's/\[\].epic//g'\` ]; then
        overall=1
    else
        exitcodeSL1=\`grep -h Exit \${logfileSL1} | sort | uniq | awk '{print \$3}'\`
        if [ \$exitcodeSL1 != 0 ]; then
    	overall=1
        fi
    fi

    logfileSL2="\`echo sl-img.o${QSUB_SPECTRAL2}.* | sed -e 's/\[\].epic//g'\`"
    if [ ! -e \`echo sl-img.o${QSUB_SPECTRAL2}.0 | sed -e 's/\[\].epic//g'\` ]; then
        overall=1
    else
        exitcodeSL2=\`grep -h Exit \${logfileSL2} | sort | uniq | awk '{print \$3}'\`
        if [ \$exitcodeSL2 != 0 ]; then
    	overall=1
        fi
    fi

    logfileSLM="makecube.o*"
    exitcodeSLM=\`grep -h Exit \${logfileSLM} | sort | uniq | awk '{print \$3}'\`
    if [ \$exitcodeSLM != 0 ]; then
	overall=1
    fi
fi

summaryfile=SUMMARY.txt
cat > \$summaryfile <<EOF_INNER
overall                  \$overall
sky-model                \$exitcodeCMODEL \$logfileCMODEL
gains-calibration        \$exitcodeCAL    \$logfileCAL
continuum-dirty          \$exitcodeCD     \$logfileCD
continuum-clean          \$exitcodeCC     \$logfileCC
contcube-dirty           \$exitcodeCCD    \$logfileCCD
contcube-dirty-makecube  \$exitcodeCCDM   \$logfileCCDM
contcube-clean           \$exitcodeCCC    \$logfileCCC
contcube-clean-makecube  \$exitcodeCCCM   \$logfileCCCM
spectral-line            \$exitcodeSL1    \$logfileSL1
spectral-line            \$exitcodeSL2    \$logfileSL2
spectral-line-makecube   \$exitcodeSLM    \$logfileSLM
analysis                 \$exitcodeAN     \$logfileAN
EOF_INNER
EOF

# Submit job
echo "Reporting script: Submitting"
unset DEPENDS
for JOB in ${GLOBAL_ALL_JOBS}; do
    if [ ! "${DEPENDS}" ]; then
        DEPENDS="afterok:${JOB}"
    else
        DEPENDS="${DEPENDS},afterok:${JOB}"
    fi
done

QEXEC_CMD="${QSUB_CMD} -W depend=${DEPENDS} ${qsubfile}"

if [ "${DRYRUN}" == "false" ]; then
    ${QEXEC_CMD}
else
    echo "    DryRun - Would have executed: ${QEXEC_CMD}" >&2
fi
