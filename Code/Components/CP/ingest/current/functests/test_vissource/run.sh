#!/bin/bash

# Run the test harness
echo "Running the testcase..."
$ASKAP_ROOT/Code/Components/CP/ingest/trunk/apps/tVisSource.sh
STATUS=$?
echo "Testcase finished"

exit $STATUS
