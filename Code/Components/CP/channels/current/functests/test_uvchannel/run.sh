#!/bin/bash

# Setup the environment
source $ASKAP_ROOT/Code/Components/CP/channels/current/init_package_env.sh

# Remove ActiveMQ data files
rm -rf $ASKAP_ROOT/3rdParty/apache-activemq/apache-activemq-5.4.1/install/data

# Start ActiveMQ
activemq start xbean:activemq-scalability.xml

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
sleep 10

# Run the uvchannel test 
echo "Running the testcase..."
$ASKAP_ROOT/Code/Components/CP/channels/current/apps/tUVChannel.sh
STATUS=$?
echo "Testcase finished"
sleep 5

# Stop ActiveMQ
activemq stop
sleep 1

exit $STATUS
