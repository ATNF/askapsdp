#!/bin/bash -l

export AIPSPATH=$ASKAP_ROOT/Code/Components/Synthesis/testdata/current

echo "Extracting 10uJy model" | tee output.out
tar zxvf ${ASKAP_ROOT}/Code/Components/Synthesis/testdata/current/simulation/models/10uJy.model.small.tgz | tee -a output.out
echo >> output.out

echo Begin csimulator `date` >> output.out
$ASKAP_ROOT/Code/Components/Synthesis/synthesis/current/install/bin/csimulator.sh -inputs simulator1.in | tee -a output1.out
$ASKAP_ROOT/Code/Components/Synthesis/synthesis/current/install/bin/csimulator.sh -inputs simulator2.in | tee -a output2.out
echo End csimulator `date` >> output.out
