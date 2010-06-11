#!/bin/bash
#
#Invoke the DataPublisher to publish some test data.
#
export CLASSPATH=$ASKAP_ROOT/lib/java-askap.jar:$CLASSPATH
java askap.datapublisher.DataPublisher DataPublisher.test localhost 4061
