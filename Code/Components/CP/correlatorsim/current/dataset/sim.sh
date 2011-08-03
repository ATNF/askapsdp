#!/bin/bash -l

export AIPSPATH=$ASKAP_ROOT/Code/Base/accessors/current

tar zxvf ${ASKAP_ROOT}/Code/Components/Synthesis/testdata/current/simulation/models/10uJy.model.small.tgz

$ASKAP_ROOT/Code/Components/Synthesis/synthesis/current/install/bin/csimulator.sh -inputs simulator1.in | tee output1.out
$ASKAP_ROOT/Code/Components/Synthesis/synthesis/current/install/bin/csimulator.sh -inputs simulator2.in | tee output2.out
