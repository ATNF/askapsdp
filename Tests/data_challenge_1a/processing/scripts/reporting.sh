##############################################################################
# Reporting the results of the data challenge run
##############################################################################

qsubfile=reporting.qsub
cat > $qsubfile <<EOF
#!/bin/bash -l
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
exitcodeCAL="-"
exitcodeCD="-"
exitcodeCC="-"
exitcodeCCD="-"
exitcodeCCC="-"
exitcodeCCDM="-"
exitcodeCCCM="-"
exitcodeSL="-"
exitcodeSLM="-"
exitcodeAN="-"

logfileSPL="split.o${QSUB_MSSPLIT}.*"
exitcodeSPL=\`grep -h Exit \${logfileSPL} | sort | uniq | awk '{print \$3}'\`
if [ \$exitcodeSPL != 0 ]; then
    overall=1
fi

if [ ${DO_CALIBRATION} == true ]; then
    logfileCMODEL="cmodel.o${QSUB_CMODEL}"
    exitcodeCMODEL=\`grep Exit \${logfileCMODEL} | awk '{print \$3}'\`
    if [ \$exitcodeCMODEL != 0 ]; then
	overall=1
    fi
    logfileCAL="ccalibrator.o${QSUB_CAL}"
    exitcodeCAL=\`grep Exit \${logfileCAL} | awk '{print \$3}'\`
    if [ \$exitcodeCAL != 0 ]; then
	overall=1
    fi
fi

if [ ${DO_CONTINUUM_DIRTY} == true ]; then
    logfileCD="cont-dirty.o${QSUB_CONTDIRTY}"
    exitcodeCD=\`grep Exit \${logfileCD} | awk '{print \$3}'\`
    if [ \$exitcodeCD != 0 ]; then
	overall=1
    fi
fi

if [ ${DO_CONTINUUM_CLEAN} == true ]; then
    logfileCC="cont-clean.o${QSUB_CONTCLEAN}"
    exitcodeCC=\`grep Exit \${logfileCC} | awk '{print \$3}'\`
    if [ \$exitcodeCC != 0 ]; then
	overall=1
    fi
fi

if [ ${DO_ANALYSIS} == true ]; then
    logfileAN="analysis.o${QSUB_ANALYSIS}"
    exitcodeAN=\`grep Exit \${logfileAN} | awk '{print \$3}'\`
    if [ \$exitcodeAN != 0 ]; then
	overall=1
    fi
fi

if [ ${DO_CONTINUUM_CUBE_DIRTY} == true ]; then
    logfileCCD="contcube-dirty.o${QSUB_CONTCUBEDIRTY}.*"
    exitcodeCCD=\`grep -h Exit \${logfileCCD} | sort | uniq | awk '{print \$3}'\`
    if [ \$exitcodeCCD != 0 ]; then
	overall=1
    fi
    logfileCCDM="makecube.o*"
    exitcodeCCDM=\`grep -h Exit \${logfileCCDM} | sort | uniq | awk '{print \$3}'\`
    if [ \$exitcodeCCDM != 0 ]; then
	overall=1
    fi
fi

if [ ${DO_CONTINUUM_CUBE_CLEAN} == true ]; then
    logfileCCC="contcube-clean.o${QSUB_CONTCUBECLEAN}.*"
    exitcodeCCC=\`grep -h Exit \${logfileCCC} | sort | uniq | awk '{print \$3}'\`
    if [ \$exitcodeCCC != 0 ]; then
	overall=1
    fi
    logfileCCCM="makecube.o*"
    exitcodeCCCM=\`grep -h Exit \${logfileCCCM} | sort | uniq | awk '{print \$3}'\`
    if [ \$exitcodeCCCM != 0 ]; then
	overall=1
    fi
fi

if [ ${DO_SPECTRAL_LINE} == true ]; then
    logfileSL1="sl-img.o${QSUB_SPECTRAL1}.*"
    exitcodeSL1=\`grep -h Exit \${logfileSL1} | sort | uniq | awk '{print \$3}'\`
    if [ \$exitcodeSL1 != 0 ]; then
	overall=1
    fi
    logfileSL2="sl-img.o${QSUB_SPECTRAL2}.*"
    exitcodeSL2=\`grep -h Exit \${logfileSL2} | sort | uniq | awk '{print \$3}'\`
    if [ \$exitcodeSL2 != 0 ]; then
	overall=1
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

    if [ "${DRYRUN}" == "false" ]; then

        # Submit the jobs
        QSUB_REPORTING=`${QSUB_CMD} -W depend=afterok:${GLOBAL_DEPEND} ${qsubfile}`

    else

	echo "Reporting script: Dry Run Only"

    fi

