#!/bin/bash -l

if [ $doCal == true ]; then

    if [ "$depend" == "" ]; then
        calDepend="-Wdepend=afterok"
    else
        calDepend=${depend}
    fi
    POINTING=0
    while [ $POINTING -lt 9 ]; do

	oldms=${inputCalMSbase}_BEAM${POINTING}.ms
	newms=${inputCalMSbase}_BEAM${POINTING}_CAL.ms

	if [ $splitMSbeforeCal == true ]; then
	    ms=${newms}
	else
	    ms=${oldms}
	fi

	mssplitParset=${parsetdir}/mssplit-BEAM${POINTING}.in
	mssplitLog=${logdir}/mssplit-BEAM${POINTING}.log
	ccalParset=${parsetdir}/ccal-BEAM${POINTING}.in
	ccalLog=${logdir}/ccal-BEAM${POINTING}.log

	calFeed=${POINTING}

	cat > ${ccalParset} <<EOF
# parameters for calibrator
Ccalibrator.dataset                             = $ms
Ccalibrator.nAnt                                = 6
Ccalibrator.nBeam                               = $calSolveNbeam
Ccalibrator.solve                               = $calSolve
Ccalibrator.interval                            = $calInterval
#
Ccalibrator.calibaccess                         = parset
Ccalibrator.calibaccess.parset                  = caldata-BEAM${POINTING}.dat
Ccalibrator.calibaccess.table                   = "caldata.tab"
Ccalibrator.calibaccess.table.maxant            = 6
Ccalibrator.calibaccess.table.maxbeam           = 9
Ccalibrator.calibaccess.table.maxchan           = 1
Ccalibrator.calibaccess.table.reuse             = false

#
Ccalibrator.sources.names                       = [field1]
Ccalibrator.sources.field1.direction	        = ${direction1934}
Ccalibrator.sources.field1.components           = src
Ccalibrator.sources.src.calibrator              = "1934-638"
#
Ccalibrator.gridder.snapshotimaging                 = true
Ccalibrator.gridder.snapshotimaging.wtolerance      = 800
Ccalibrator.gridder                                 = ${calGridder}
Ccalibrator.gridder.${calGridder}.wmax                  = ${wmaxCal}
Ccalibrator.gridder.${calGridder}.nwplanes              = ${nwCal}
Ccalibrator.gridder.${calGridder}.oversample            = ${osCal}
Ccalibrator.gridder.${calGridder}.diameter              = 12m
Ccalibrator.gridder.${calGridder}.blockage              = 2m
Ccalibrator.gridder.${calGridder}.maxfeeds              = 9
Ccalibrator.gridder.${calGridder}.maxsupport            = 512
Ccalibrator.gridder.${calGridder}.variablesupport       = true
Ccalibrator.gridder.${calGridder}.offsetsupport         = true
Ccalibrator.gridder.${calGridder}.frequencydependent    = true

Ccalibrator.ncycles                             = $ncycCal

EOF

	qsubfile=ccal_BEAM${POINTING}.qsub
	cat > $qsubfile <<EOF
#!/bin/bash -l
#PBS -l walltime=01:00:00
#PBS -l mppwidth=1
#PBS -l mppnppn=1
#PBS -N ccal${POINTING}
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

cd \$PBS_O_WORKDIR

mssplit=${mssplit}
ccal=${ccal}

rm -rf $newms

flagRunSplit=${splitMSbeforeCal}
if [ \$flagRunSplit == true ]; then
    echo "Running mssplit to get MS with only feed ${feedlist[$POINTING]} in it"
    cat > ${mssplitParset} <<EOFINNER
vis = ${oldms}
outputvis = ${newms}
beams = [${POINTING}]
channel = 1
EOFINNER
    aprun -n 1 -N 1 \${mssplit} -c ${mssplitParset} > ${mssplitLog}
fi

aprun -n 1 -N 1 \${ccal} -c ${ccalParset} > ${ccalLog}

EOF

	if [ $doSubmit == true ]; then

	    latestID=`qsub $depend $qsubfile`
	    calDepend="${calDepend}:${latestID}"

	    echo "Running ccalibrator for pointing ${POINTING}, producing measurement set ${newms}: ID=${latestID}"

	fi

	POINTING=`expr $POINTING + 1`

    done

fi
