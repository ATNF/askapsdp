#!/bin/bash

mpirun -np 7 $ASKAP_ROOT/Code/Components/CP/pipelinetasks/current/apps/cmodel.sh -inputs cmodel.in
