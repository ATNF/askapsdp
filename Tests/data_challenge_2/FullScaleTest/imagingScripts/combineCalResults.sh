#!/bin/bash -l

sbatchfile=combineCaldata.sbatch
cat > $sbatchfile <<EOF
#SBATCH --time=01:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name combineCal
#SBATCH --mail-type=ALL
#SBATCH --export=ASKAP_ROOT,AIPSPATH

rm -f caldata-combined.dat
N=0
while [ \$N -lt 9 ]; do
    grep "\\.\${N} = " caldata-BEAM\${N}.dat >> caldata-combined.dat
    N=\`expr \$N + 1\`
done

EOF

if [ $doSubmit == true ]; then
    latestID=`sbatch $calDepend $sbatchfile | awk '{print $4}'`
    calDepend="${calDepend}:${latestID}"
fi

