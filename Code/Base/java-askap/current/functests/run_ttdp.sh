#!/bin/bash
#
#Invoke the TimeTaggedDataPublisher to publish some test data.
#
export CLASSPATH=$ASKAP_ROOT/lib/java-askap.jar:$CLASSPATH
java askap.datapublisher.TimeTaggedDataPublisher TimeTaggedDataPublisher.test localhost 4061
