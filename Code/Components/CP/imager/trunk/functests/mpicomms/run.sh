#!/bin/bash

OUTPUT=output.txt

export AIPSPATH=${ASKAP_ROOT}/Code/Components/Synthesis/testdata/trunk

if [ ! -x ${ASKAP_ROOT}/Code/Components/CP/imager/trunk/apps/tMpiComms.sh ]; then
    echo tMpiComms.sh does not exit
fi

mpirun -np 3 ${ASKAP_ROOT}/Code/Components/CP/imager/trunk/apps/tMpiComms.sh | tee $OUTPUT
if [ $? -ne 0 ]; then
    echo Error: mpirun returned an error
    exit 1
fi
