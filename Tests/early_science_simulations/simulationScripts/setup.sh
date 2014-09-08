#!/usr/bin/bash

#####
## Makes the working directory, moves to that directory, then makes
## all required subdirectories: parsets, logs, MS, chunks & slices

workdir="run_${now}"

echo "Making working directory $workdir"
mkdir -p ${workdir}
cd ${workdir}
mkdir -p ${parsetdir}
mkdir -p ${logdir}
mkdir -p ${msdir}
mkdir -p ${chunkdir}
mkdir -p ${slicedir}
