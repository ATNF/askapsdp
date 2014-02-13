#!/bin/bash -l


flagRunSplit=false

if [ $doCal == true ]; then

    . ${scriptdir}/getTags.sh

    ccalParset=parsets/ccal-${POINTING}.in
    ccalLog=logs/ccal-${now}-${POINTING}.log

    newms=${msbase}_CALIBRATED_${POINTING}.ms
    oldms=${msbase}_CORRUPTED_${POINTING}.ms
    
    mssplitParset=parsets/mssplit-${POINTING}.in
    mssplitLog=logs/mssplit-${POINTING}.log

    echo "Running ccalibrator for pointing ${POINTING}, producing measurement set ${newms}"

    ncycCal=10
    calInterval=1800
    calibparset=rndgains.in
    calSolveNbeam=9
    calFeed=${feedlist[${POINTING}]}
    calSolve=gains
    #    calDirection=${dirlist[${POINTING}]}
    #    calDirection=${dirlist[${feedlist[${POINTING}]}]}
    calDirection=${dirlist[4]}
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
Ccalibrator.calibaccess.parset                  = result.dat
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
Ccalibrator.gridder                                 = AWProject
Ccalibrator.gridder.AWProject.wmax                  = 800
Ccalibrator.gridder.AWProject.nwplanes              = 129
Ccalibrator.gridder.AWProject.oversample            = 4
Ccalibrator.gridder.AWProject.diameter              = 12m
Ccalibrator.gridder.AWProject.blockage              = 2m
Ccalibrator.gridder.AWProject.maxfeeds              = 9
Ccalibrator.gridder.AWProject.maxsupport            = 512
Ccalibrator.gridder.AWProject.variablesupport       = true
Ccalibrator.gridder.AWProject.offsetsupport         = true
Ccalibrator.gridder.AWProject.frequencydependent    = true

Ccalibrator.ncycles                             = $ncycCal

EOF

    qsubfile=ccal_${tags}.qsub
    cat > $qsubfile <<EOF
#!/bin/bash -l
#PBS -l walltime=01:00:00
#PBS -l mppwidth=19
#PBS -l mppnppn=19
#PBS -N ccal${POINTING}
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

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

fi
