#!/bin/bash -l

#Build calc
unset ASKAP_ROOT
cd $WORKSPACE/trunk
/usr/bin/python2.6 bootstrap.py
source initaskap.sh
rbuild -n Code/Components/TOM/tests/test_calc/current -t install
if (( $? )); then
    echo "calc build FAILED"
    exit 1
fi
# Run the functests
rbuild -n Code/Components/TOM/tests/test_calc/current -t functest
