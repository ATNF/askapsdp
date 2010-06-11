#!/bin/bash

source ../../init_package_env.sh

# Start ActiveMQ
echo "Starting ActiveMQ..."
activemq > activemq.log 2>&1 &
ACTIVEMQ_PID=$!
sleep 5
echo "Starting ActiveMQ...DONE"

# Run the test
../../apps/tMQBrokerSingleton.sh

# Stop ActiveMQ
echo -n "Stopping ActiveMQ..."
kill $ACTIVEMQ_PID > /dev/null 2>&1
wait $ACTIVEMQ_PID
echo STOPPED

