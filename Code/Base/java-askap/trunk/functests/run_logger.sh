#!/bin/bash
#
#Invoke the askap.logger.Logger class so that it generates some log messages
#using the LoggerIce implementation.
#
#Really want to avoid populating the CLASSPATH in this way but I am not sure
#how else to go about it at the moment.
#

java -cp ../install/lib/java-askap.jar:../../../../../lib/Ice-3.3.0.jar -Daskap.logger.configuration=askaplogger-ice-locator-localhost-4061.properties askap.logger.Logger
