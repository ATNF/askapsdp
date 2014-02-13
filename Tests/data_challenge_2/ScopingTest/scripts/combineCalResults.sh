#!/bin/bash -l

qsubfile=combineCaldata.qsub
cat > $qsubfile <<EOF
#PBS -l walltime=01:00:00
#PBS -l mppwidth=1
#PBS -l mppnppn=1
#PBS -N combineCal
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

cd \$PBS_O_WORKDIR

rm -f caldata-combined.dat
N=0
while [ $N -le 9 ]; do
    grep ".${N} = " caldata-${N}.dat >> caldata-combined.dat
    N=`expr $N + 1`
done

EOF

latestID=`qsub $depend $qsubfile`
