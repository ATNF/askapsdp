#!/bin/bash -l


. ${scriptdir}/getTags.sh

if [ $doCal == true ]; then
    mstag="CORRUPTED"
fi

selection="# No feed selection"
if [ $doTrim == true ]; then
    imtag=${imtag}.${POINTING}
    tag=${tag}${POINTING}
    selection="Cimager.Feed = ${POINTING}"
fi

imParset=parsets/cim-Science-${tag}.in
imLogfile=logs/cim-Science-${tag}.log

ms=${msbase}_Science_${mstag}.ms
image=image.i.dirty.science.${imtag}

freq="${nurefMHz}e6"
direction=${dirlist[4]}

cat > ${imParset} << EOF_INNER
Cimager.dataset                                 = ${ms}
${selection}
#
Cimager.Images.Names                            = [${image}]
Cimager.Images.shape                            = [2048,2048]
Cimager.Images.cellsize                         = [10arcsec,10arcsec]
Cimager.Images.${image}.frequency          = [${freq},${freq}]
Cimager.Images.${image}.nchan              = 1
#Cimager.Images.${image}.direction          = [12h30m00.00, -45.00.00.00, J2000]
Cimager.Images.${image}.direction          = ${direction}
#
Cimager.gridder.snapshotimaging                 = true
Cimager.gridder.snapshotimaging.wtolerance      = 800
Cimager.gridder                                 = ${imGridder}
Cimager.gridder.${imGridder}.wmax                  = 800
Cimager.gridder.${imGridder}.nwplanes              = ${nw}
Cimager.gridder.${imGridder}.oversample            = ${os}
Cimager.gridder.${imGridder}.diameter              = 12m
Cimager.gridder.${imGridder}.blockage              = 2m
Cimager.gridder.${imGridder}.maxfeeds              = 36
Cimager.gridder.${imGridder}.maxsupport            = 512
Cimager.gridder.${imGridder}.variablesupport       = true
Cimager.gridder.${imGridder}.offsetsupport         = true
Cimager.gridder.${imGridder}.frequencydependent    = true
#
Cimager.solver                                  = Dirty
Cimager.solver.Dirty.tolerance                  = 0.1
Cimager.solver.Dirty.verbose                    = True
Cimager.ncycles                                 = 0

Cimager.preconditioner.Names                    = [Wiener, GaussianTaper]
Cimager.preconditioner.GaussianTaper            = [30arcsec, 30arcsec, 0deg]
Cimager.preconditioner.Wiener.robustness        = 0.0
Cimager.preconditioner.Wiener.taper             = 64
#
Cimager.restore                                 = true
Cimager.restore.beam                            = fit
#Cimager.restore.equalise                        = True
#
# Apply calibration
Cimager.calibrate                               = ${doCal}
Cimager.calibaccess                             = parset
Cimager.calibaccess.parset                      = ${calParams}
Cimager.calibrate.scalenoise                    = true
Cimager.calibrate.allowflag                     = true
EOF_INNER

slurmtag="cimSci${tag}"
sbatchfile=cim_Science_${tag}.sbatch
cat > $sbatchfile <<EOF
#!/bin/bash -l
#SBATCH --time=01:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name ${slurmtag}
#SBATCH --export=ASKAP_ROOT,AIPSPATH

aprun ${cim} -c ${imParset} > ${imLogfile}

EOF

latestID=`sbatch $depend $sbatchfile | awk '{print $4}'`


echo "Running cimager for science field, imaging ${ms} to create ${image}.restored: ID=${latestID}"


