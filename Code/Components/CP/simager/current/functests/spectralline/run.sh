#!/bin/bash

OUTPUT=output.txt

export AIPSPATH=${ASKAP_ROOT}/Code/Base/accessors/current

if [ ! -x ../../apps/simager.sh ]; then
    echo simager.sh does not exit
fi

IMAGE=image.i.cube.spectral
PSF=psf.i.cube.spectral
WEIGHTS=weights.i.cube.spectral

echo -n "Removing image cubes..."
rm -rf $IMAGE
rm -rf $PSF
rm -rf WEIGHTS

echo Done

echo -n Extracting measurement set...
tar zxf ../10uJy_stdtest.ms.tgz
echo Done

mpirun -np 3 ../../apps/simager.sh -c simager.in | tee $OUTPUT
if [ $? -ne 0 ]; then
    echo Error: mpirun returned an error
    exit 1
fi

echo -n Removing measurement set...
rm -rf 10uJy_stdtest.ms
echo Done

# Check for instances of "Askap error"
grep -c "Askap error" $OUTPUT > /dev/null
if [ $? -ne 1 ]; then
    echo "Askap error reported in output.txt"
    exit 1
fi

# Check for instances of "Unexpected exception"
grep -c "Unexpected exception" $OUTPUT > /dev/null
if [ $? -ne 1 ]; then
    echo "Exception reported in output.txt"
    exit 1
fi

# Check for the existance of the various image cubes
if [ ! -d ${IMAGE} ]; then
    echo "Error ${IMAGE} not created"
    exit 1
fi

if [ ! -d ${PSF}${IDX} ]; then
    echo "Error ${PSF} not created"
    exit 1
fi

if [ ! -d ${WEIGHTS}${IDX} ]; then
    echo "Error ${WEIGHTS} not created"
    exit 1
fi

echo Done

