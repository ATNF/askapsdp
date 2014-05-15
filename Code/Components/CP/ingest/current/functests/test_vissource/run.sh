#!/bin/bash

cd `dirname $0`

# Run the test harness
echo "Running the testcase..."
../../apps/tVisSource.sh
STATUS=$?
echo "Testcase finished"

exit $STATUS
