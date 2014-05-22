#!/bin/bash -l

sbatchfile=combineCaldata.sbatch
calParams=caldata-combined.dat
cat > $sbatchfile <<EOF
#SBATCH --time=01:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name combineCal
#SBATCH --mail-type=ALL
#SBATCH --export=ASKAP_ROOT,AIPSPATH

rm -f caldata-combined.dat
N=0
while [ \$N -le 9 ]; do
    grep "\\.\${N} = " caldata-\${N}.dat >> ${calParams}
    N=\`expr \$N + 1\`
done

EOF

latestID=`sbatch $depend $sbatchfile | awk '{print $4}'`
