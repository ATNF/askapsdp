#!/bin/bash

# Setup the environment
source $ASKAP_ROOT/Code/Components/CP/eventchannel/current/init_package_env.sh

# Start ActiveMQ
activemq start 

# Wait until it is responsive
MQSTATUS=1
COUNT=0
while [[ $MQSTATUS -ne 0 && $COUNT -ne 10 ]]; do
    activemq status > /dev/null
    MQSTATUS=$?
    COUNT=`expr $COUNT + 1`
    sleep 1
done

if [ $COUNT -eq 10 ]; then
    activemq stop
    echo "ActiveMQ failed to start"
    exit 1
fi

# Sleep a little more
sleep 5

# Run the ingest pipeline
echo "Running the testcase..."
$ASKAP_ROOT/Code/Components/CP/eventchannel/current/apps/tEventChannel.sh
STATUS=$?
echo "Testcase finished"

# Stop ActiveMQ
activemq stop
sleep 2

exit $STATUS
