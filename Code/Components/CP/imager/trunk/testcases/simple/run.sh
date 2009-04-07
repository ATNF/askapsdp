#!/bin/bash

echo -n Removing images...
rm -rf image.i.10uJy_dirty_stdtest/ psf.i.10uJy_dirty_stdtest/ weights.i.10uJy_dirty_stdtest/
echo Done

echo -n Extracting measurement set...
tar zxf 10uJy_stdtest.ms.tgz
mv 10uJy_stdtest.ms 10uJy_stdtest_0.ms
tar zxf 10uJy_stdtest.ms.tgz
mv 10uJy_stdtest.ms 10uJy_stdtest_1.ms
echo Done

mpirun -np 2 ${ASKAP_ROOT}/Code/Components/CP/imager/trunk/apps/imager.sh -inputs dirty.in

echo -n Removing measurement set...
rm -rf 10uJy_stdtest_0.ms
rm -rf 10uJy_stdtest_1.ms
echo Done

