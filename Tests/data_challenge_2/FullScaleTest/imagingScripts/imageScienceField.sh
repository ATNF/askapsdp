#!/bin/bash -l


POINTING=0
while [ $POINTING -lt 9 ]; do
    . ${imScripts}/imageScienceFieldBeam.sh
    imdepend="${imdepend}:${latestID}"
    POINTING=`expr $POINTING + 1`
done

range="BEAM0..8"

if [ $doMFS == true ]; then
    nterms="linmos.nterms     = 2"
else
    nterms="# no nterms parameter since not MFS"
fi

if [ ${IMAGING_GRIDDER} == "AWProject" ]; then
    weightingPars="# Use the weight images directly:
linmos.weighttype = FromWeightImages
"
else
    if [ ${model} == "SKADS" ]; then
	weightingPars="# Use primary beam models at specific positions:
linmos.weighttype    = FromPrimaryBeamModel
linmos.weightstate   = Inherent
linmos.feeds.centre  = [12h30m00.00, -45.00.00.00]
linmos.feeds.spacing = 1deg
linmos.feeds.BEAM0   = [-1.0, -1.0]
linmos.feeds.BEAM1   = [-1.0,  0.0]
linmos.feeds.BEAM2   = [-1.0,  1.0]
linmos.feeds.BEAM3   = [ 0.0, -1.0]
linmos.feeds.BEAM4   = [ 0.0,  0.0]
linmos.feeds.BEAM5   = [ 0.0,  1.0]
linmos.feeds.BEAM6   = [ 1.0, -1.0]
linmos.feeds.BEAM7   = [ 1.0,  0.0]
linmos.feeds.BEAM8   = [ 1.0,  1.0]
"
    else
	weightingPars="# Use primary beam models at specific positions:
linmos.weighttype    = FromPrimaryBeamModel
linmos.weightstate   = Inherent
linmos.feeds.centre  = [12h30m00.00, -45.00.00.00]
linmos.feeds.spacing = 1deg
linmos.feeds.BEAM0   = [0,0]
linmos.feeds.BEAM1   = [-0.572425, 0.947258]
linmos.feeds.BEAM2   = [-1.14485, 1.89452]
linmos.feeds.BEAM3   = [0.572425, -0.947258]
linmos.feeds.BEAM4   = [-1.23347, -0.0987957]
linmos.feeds.BEAM5   = [-1.8059, 0.848462]
linmos.feeds.BEAM6   = [0.661046, 1.04605]
linmos.feeds.BEAM7   = [0.0886209, 1.99331]
linmos.feeds.BEAM8   = [1.23347, 0.0987957]
"
    fi
fi

linmosqsub=linmosFull.qsub
cat > $linmosqsub <<EOF
#!/bin/bash -l
#PBS -l walltime=01:00:00
#PBS -l mppwidth=1
#PBS -l mppnppn=1
#PBS -N linmos
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

cd \$PBS_O_WORKDIR

linmos=\${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/linmos.sh

parset=parsets/linmos_${type}_\${PBS_JOBID}.in
cat > \${parset} <<EOF_INNER
linmos.names       = [${range}]
linmos.findmosaics = true
linmos.psfref      = 4
${weightingPars}
${nterms}
EOF_INNER
log=logs/linmos_${type}_\${PBS_JOBID}.log

aprun \${linmos} -c \${parset} > \${log}


EOF

if [ $doSubmit == true ]; then 
    ID=`qsub ${imdepend} ${linmosqsub}`
    echo "Have submitted a linmos job with ID=${ID}, via 'qsub ${imdepend} ${linmosqsub}'"
fi
