#!/bin/bash -l


flagRunSplit=false

if [ $doCal == true ]; then

    . ${scriptdir}/getTags.sh

    ccalParset=parsets/ccal-${tag}-${POINTING}.in
    ccalLog=logs/ccal-${tag}-${POINTING}.log

    newms=${msbase}_CALIBRATED_${POINTING}.ms
    oldms=${msbase}_CORRUPTED_${POINTING}.ms
    
    mssplitParset=parsets/mssplit-${tag}-${POINTING}.in
    mssplitLog=logs/mssplit-${tag}-${POINTING}.log

    ncycCal=10
    calInterval=1800
    calibparset=rndgains.in
    calSolveNbeam=9
#    calFeed=${feedlist[${POINTING}]}
    calFeed=${POINTING}
    calSolve=gains
    #    calDirection=${dirlist[${POINTING}]}
    #    calDirection=${dirlist[${feedlist[${POINTING}]}]}
#    calDirection=${baseDirection}
    calDirection=${direction1934}
    #    calModel="Ccalibrator.sources.src.flux.i                  = `echo ${sourceFlux} | awk '{print $1/2.}'`
    calModel="Ccalibrator.sources.src.calibrator  = \"1934-638\""
    ##     calModel="Ccalibrator.sources.src.flux.i                  = ${sourceFlux}
    ## Ccalibrator.sources.src.direction.ra            = 0
    ## Ccalibrator.sources.src.direction.dec           = 0"
    ## #Ccalibrator.sources.src.direction.ra            = `echo ${raOff[${POINTING}]} ${radian} | awk '{print $1/$2}'`
    ## #Ccalibrator.sources.src.direction.dec           = `echo ${decOff[${POINTING}]} $radian | awk '{print $1/$2}'`"


    cat > ${ccalParset} <<EOF
# parameters for calibrator
Ccalibrator.dataset                             = $newms
#Ccalibrator.Feed                                = ${calFeed}
Ccalibrator.nAnt                                = 6
Ccalibrator.nBeam                               = $calSolveNbeam
Ccalibrator.solve                               = $calSolve
#Ccalibrator.refgain                             = gain.g11.0.0
#Ccalibrator.refgain                             = gain.g11.${calFeed}.${calFeed}
Ccalibrator.interval                            = $calInterval
#
Ccalibrator.calibaccess                         = parset
Ccalibrator.calibaccess.parset                  = caldata-${POINTING}.dat
Ccalibrator.calibaccess.table                   = "caldata.tab"
Ccalibrator.calibaccess.table.maxant            = 6
Ccalibrator.calibaccess.table.maxbeam           = 9
Ccalibrator.calibaccess.table.maxchan           = 1
Ccalibrator.calibaccess.table.reuse             = false

#
Ccalibrator.sources.names                       = [field1]
Ccalibrator.sources.field1.direction	        = $calDirection
Ccalibrator.sources.field1.components           = src
${calModel}

Ccalibrator.gridder.snapshotimaging                 = true
Ccalibrator.gridder.snapshotimaging.wtolerance      = 800
Ccalibrator.gridder                                 = ${calGridder}
Ccalibrator.gridder.${calGridder}.wmax                  = 800
Ccalibrator.gridder.${calGridder}.nwplanes              = 129
Ccalibrator.gridder.${calGridder}.oversample            = 4
Ccalibrator.gridder.${calGridder}.diameter              = 12m
Ccalibrator.gridder.${calGridder}.blockage              = 2m
Ccalibrator.gridder.${calGridder}.maxfeeds              = 9
Ccalibrator.gridder.${calGridder}.maxsupport            = 512
Ccalibrator.gridder.${calGridder}.variablesupport       = true
Ccalibrator.gridder.${calGridder}.offsetsupport         = true
Ccalibrator.gridder.${calGridder}.frequencydependent    = true

Ccalibrator.ncycles                             = $ncycCal

EOF

    qsubfile=ccal_${POINTING}_${tag}.qsub
    cat > $qsubfile <<EOF
#!/bin/bash -l
#PBS -l walltime=01:00:00
#PBS -l mppwidth=1
#PBS -l mppnppn=1
#PBS -N ccal${POINTING}${tag}
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

cd \$PBS_O_WORKDIR

rm -rf $newms

flagRunSplit=${flagRunSplit}
if [ \$flagRunSplit == true ]; then
    echo "Running mssplit to get MS with only feed ${feedlist[$POINTING]} in it"
    cat > ${mssplitParset} <<EOFINNER
vis = ${oldms}
outputvis = ${newms}
beams = [${feedlist[${POINTING}]}]
channel = 1
EOFINNER
    aprun ${mssplit} -c ${mssplitParset} > ${mssplitLog}
else
    cp -R $oldms $newms
fi

aprun ${ccal} -c ${ccalParset} > ${ccalLog}

EOF

    latestID=`qsub $depend $qsubfile`

    echo "Running ccalibrator for pointing ${POINTING}, producing measurement set ${newms}: ID=${latestID}"


fi
