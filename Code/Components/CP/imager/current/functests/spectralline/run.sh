#!/bin/bash

OUTPUT=output.txt

export AIPSPATH=${ASKAP_ROOT}/Code/Base/accessors/current

if [ ! -x ../../apps/imager.sh ]; then
    echo imager.sh does not exit
fi

IMAGE=image.i.10uJy_spectralline_ch
PSF=psf.i.10uJy_spectralline_ch
WEIGHTS=weights.i.10uJy_spectralline_ch

echo -n Removing images...
IDX=1
while [ $IDX -le 32 ] ; do
    rm -rf ${IMAGE}${IDX}
    rm -rf ${PSF}${IDX}
    rm -rf ${WEIGHTS}${IDX}
    IDX=`expr $IDX + 1`
done

echo Done

echo -n Extracting measurement set...
tar zxf ../10uJy_stdtest.ms.tgz
mv -f 10uJy_stdtest.ms 10uJy_stdtest_0.ms
tar zxf ../10uJy_stdtest.ms.tgz
mv -f 10uJy_stdtest.ms 10uJy_stdtest_1.ms
echo Done

mpirun -np 3 ../../apps/imager.sh -inputs imager.in | tee $OUTPUT
if [ $? -ne 0 ]; then
    echo Error: mpirun returned an error
    exit 1
fi

echo -n Removing measurement set...
rm -rf 10uJy_stdtest_0.ms
rm -rf 10uJy_stdtest_1.ms
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

# Check for the existance of the various image files
IDX=1
while [ $IDX -le 32 ] ; do
    if [ ! -d ${IMAGE}${IDX} ]; then
        echo "Error ${IMAGE}${IDX} not created"
        exit 1
    fi

    if [ ! -d ${PSF}${IDX} ]; then
        echo "Error ${PSF}${IDX} not created"
        exit 1
    fi

    if [ ! -d ${WEIGHTS}${IDX} ]; then
        echo "Error ${WEIGHTS}${IDX} not created"
        exit 1
    fi
    IDX=`expr $IDX + 1`
done

echo Done

