##############################################################################
# Making a spectral cube
##############################################################################


#####################
#  Script to combine individual channel images into a single cube
#   - makes use of the CP/imager/current/apps/makecube utility
#   - requires the following environment variables to be set:
#     * DODELETE - whether to delete all but the first channel images
#     * IMAGEBASE - the constant (base) part of the image name. If doing the restored image, don't worry about the .restored flag, just use ISRESTORED=true
#     * NUMCH - the number of channel images
#     * OUTPUTCUBE - the name of the final cube
#     * ISRESTORED - if true, we are combining \${IMAGEBASE}_chN.restored images (and so need to rename them prior to running makecube)
#   - input images must be of the form \${IMAGEBASE}_chN, where N=1...\$NUMCH
#   - the final cube will have its spectral coordinates corrected by a CASA script
#####################


if [ ${IMAGEBASE} == "" ]; then
    echo "Have not set \$IMAGEBASE, so not running make-spectral-cube."
else

    qsubfile=make-spectral-cube--${OUTPUTCUBE}.qsub
    cat > ${qsubfile} <<EOF
#!/bin/bash -l
#PBS -W group_list=${QUEUEGROUP}
#PBS -l walltime=06:00:00
#PBS -l select=1:ncpus=1:mem=2GB:mpiprocs=1
##PBS -M first.last@csiro.au
#PBS -N makecube
#PBS -m bea
#PBS -j oe

cd \$PBS_O_WORKDIR

makecube=${ASKAP_ROOT}/Code/Components/CP/imager/current/install/bin/makecube

outfile=${LOGDIR}/makecube-\${PBS_JOBID}.log

echo Begin makecube \`date\` > \$outfile
echo Using image base name ${IMAGEBASE}, with ${NUMCH} channels, to create ${OUTPUTCUBE} >> \$outfile

doDelete=$DODELETE
isRestored=$ISRESTORED

if [ "$IMAGEBASE" == "" ]; then
    echo "Have not set \\\$IMAGEBASE. Exiting."
    exit 1
fi

if [ "$OUTPUTCUBE" == "" ]; then
    echo "Have not set \\\$OUTPUTCUBE. Exiting."
    exit 1
fi

if [ \$isRestored == true ]; then
    base=temp
else
    base=${IMAGEBASE}
fi

# Test for existence of the images, and rename if doing restored images
C=1
while [ \$C -le ${NUMCH} ]; do
    if [ ! -e ${IMAGEBASE}_ch\${C} ]; then
	echo "Could not find image ${IMAGEBASE}_ch\${C}. Exiting."
    fi
    if [ \$isRestored == true ]; then
	mv ${IMAGEBASE}_ch\${C}.restored \${base}_ch\${C}
    fi
    C=\`expr \$C + 1\`
done

# Run makecube
\$makecube \${base}_ch ${NUMCH} ${OUTPUTCUBE} | tee -a \$outfile
err=\$?
if [ \$err != 0 ]; then
    echo "Makecube failed with error \$err"
    exit \$err
fi

# Delete images if required, and rename back if doing restored image
if [ \$doDelete == true ]; then
    if [ \$isRestored == true ]; then
	mv \${base}_ch1 ${IMAGEBASE}_ch1.restored
    fi
    C=2
    while [ \$C -le \$NUMCH ]; do
        /bin/rm -rf \${base}_ch\${C}
        C=\`expr \$C + 1\`
    done
else
    C=1
    if [ \$isRestored == true ]; then
	while [ \$C -le $NUMCH ]; do
	    mv \${base}_ch\${C} ${IMAGEBASE}_ch\${C}
	    C=\`expr \$C + 1\`
	done
    fi
fi


pyfile=fixCube-\${PBS_JOBID}.py
cat > \${pyfile} <<EOF_INNER
#!/bin/env python

######
# This is a script to be run in casapy.
# It fixes the spectral coordinates of a cube created by makecube,
#  and sets the restoring beam to be the same as the first channel.
#  (this is interesting - it will be different for different channels...)

ia.open('${IMAGEBASE}_ch1.restored')
beam=ia.restoringbeam()
ia.close()

if(ia.open('${OUTPUTCUBE}')):
    
    csys=ia.coordsys()
    #    csys.setrestfrequency(1420405751.786)
    crec=csys.torecord()
    crec['spectral2']['wcs']['cdelt']=-1.e6
    crec['spectral2']['wcs']['crpix']=0.
    crec['spectral2']['wcs']['crval']=1421.e6

    ia.setcoordsys(crec)

    if(name[-9:] == '.restored'):
        ia.setrestoringbeam(beam=beam)
    
    ia.close()

EOF_INNER

if [ -e ${OUTPUTCUBE} ]; then
    casapy --nologger --log2term -c \${pyfile}
    err=\$?
    exit \$err
else
    echo "ERROR creating output cube ${OUTPUTCUBE}
    exit 1
fi

EOF

    if [ "${DRYRUN}" == "false" ]; then

        # Submit the jobs
	if [ "${DEPENDS}" ]; then
            QSUB_MAKECUBE=`${QSUB_CMD} ${DEPENDS} ${qsubfile}`
	else
            QSUB_MAKECUBE=`${QSUB_CMD} ${qsubfile}`
            QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_MAKECUBE}"
	fi

    else

	echo "Makecube utility (${OUTPUTCUBE}): Dry Run Only"

    fi


fi

