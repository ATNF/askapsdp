#!/bin/bash

export AIPSPATH=${ASKAP_ROOT}/Code/Components/Synthesis/testdata/trunk

if [ ! -x ${ASKAP_ROOT}/Code/Components/CP/imager/branches/specline-dev/apps/imager.sh ]; then
    echo imager.sh does not exit
fi

echo -n Removing images...
rm -rf image.i.10uJy_spectralline_ch* psf.i.10uJy_spectralline_ch* weights.i.10uJy_spectralline_ch*
echo Done

echo -n Extracting measurement set...
tar zxf 10uJy_stdtest.ms.tgz
mv 10uJy_stdtest.ms 10uJy_stdtest_0.ms
tar zxf 10uJy_stdtest.ms.tgz
mv 10uJy_stdtest.ms 10uJy_stdtest_1.ms
echo Done

mpirun -np 2 ${ASKAP_ROOT}/Code/Components/CP/imager/branches/specline-dev/apps/imager.sh -inputs imager.in

echo -n Removing measurement set...
rm -rf 10uJy_stdtest_0.ms
rm -rf 10uJy_stdtest_1.ms
echo Done

if [ ! -d image.i.10uJy_dirty_stdtest ]; then
    echo Image file was not created
fi

if [ ! -d psf.i.10uJy_dirty_stdtest ]; then
    echo PSF file was not created
fi

if [ ! -d weights.i.10uJy_dirty_stdtest ]; then
    echo Weights image was not created
fi
