#!/bin/bash
#
#Invoke the askap.logger.Logger class so that it generates some log messages
#using the LoggerIce implementation.
#
export CLASSPATH=$ASKAP_ROOT/lib/java-askap.jar:$CLASSPATH
java askap.datapublisher.DataPublisher DataPublisher.test localhost 4061
