#!/bin/bash
#
#Invoke the askap.logger.Logger class so that it generates some log messages
#using the LoggerIce implementation.
#
#Really want to avoid populating the CLASSPATH in this way but I am not sure
#how else to go about it at the moment.
#
export CLASSPATH=$ASKAP_ROOT/lib/java-askap.jar:$CLASSPATH
java -Daskap.logger.configuration=askaplogger-ice-locator-localhost-4061.properties askap.logger.Logger
