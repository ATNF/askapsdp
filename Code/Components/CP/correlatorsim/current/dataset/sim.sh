#!/bin/bash -l

export AIPSPATH=$ASKAP_ROOT/Code/Components/Synthesis/testdata/trunk

echo "Extracting 10uJy model" | tee output.out
tar zxvf ${ASKAP_ROOT}/Code/Components/Synthesis/testdata/trunk/simulation/models/10uJy.model.small.tgz | tee -a output.out
echo >> output.out

echo Begin csimulator `date` >> output.out
$ASKAP_ROOT/Code/Components/Synthesis/synthesis/trunk/install/bin/csimulator.sh -inputs simulator.in | tee -a output.out
echo End csimulator `date` >> output.out
