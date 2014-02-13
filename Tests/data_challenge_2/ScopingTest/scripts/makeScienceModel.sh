#!/bin/bash -l 

crParset=parsets/create_scienceModel.in
crLog=logs/createfits_scienceModel.log

sourcelist=parsets/srclist_scienceModel.txt
cat > $sourcelist <<EOF
187.6 -44.0 5. 0. 0. 0. 0. 0.
186.10984 -44.1 5. 0. 0. 0. 0. 0.
188.79016 -44.0 5. 0. 0. 0. 0. 0.
187.5 -44.9 5. 0. 0. 0. 0. 0.
186.08579 -45.0 5. 0. 0. 0. 0. 0.
189.01421 -45.0 5. 0. 0. 0. 0. 0.
187.5 -46.1 5. 0. 0. 0. 0. 0.
186.16044 -46.0 5. 0. 0. 0. 0. 0.
188.93956 -45.9 5. 0. 0. 0. 0. 0.
EOF

rm -rf $modelImage

cat >> ${crParset} <<EOF
createFITS.filename         = !${modelImage}.fits
createFITS.casaOutput       = true
createFITS.fitsOutput       = false
createFITS.flagWriteByChannel = true
createFITS.sourcelist       = ${sourcelist}
createFITS.database         = Continuum
createFITS.sourcelisttype   = continuum
createFITS.useGaussians     = true
createFITS.verboseSources   = false
createFITS.posType          = deg
createFITS.bunit            = Jy/pixel
createFITS.dim              = 4
createFITS.axes             = [${npix}, ${npix}, ${nstokes}, 1]
createFITS.WCSimage.ctype   = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
createFITS.WCSimage.cunit   = [deg, deg, "", Hz]
createFITS.WCSimage.crval   = [${ra}, ${dec}, ${stokesZero}, ${rfreq}]
createFITS.WCSimage.crpix   = [${rpix}, ${rpix}, ${rstokes}, ${rchan}]
createFITS.WCSimage.crota   = [0., 0., 0., 0.]
createFITS.WCSimage.cdelt   = [-${delt}, ${delt}, ${dstokes}, ${chanw}]
createFITS.WCSsources       = false
createFITS.addNoise         = false
createFITS.doConvolution    = false
createFITS.baseFreq         = ${basefreq}
createFITS.flagSpectralInfo = false
createFITS.PAunits          = rad
createFITS.minMinorAxis     = 0.000100
EOF

qsubfile=createScienceModel.qsub
cat > $qsubfile <<EOF
#!/bin/bash -l
#PBS -l walltime=01:00:00
#PBS -l mppwidth=1
#PBS -l mppnppn=1
#PBS -N crSci
#PBS -m a
#PBS -j oe
#PBS -v ASKAP_ROOT,AIPSPATH

cd \$PBS_O_WORKDIR

aprun $createFITS -c $crParset > $crLog

EOF


latestID=`qsub $depend $qsubfile`

