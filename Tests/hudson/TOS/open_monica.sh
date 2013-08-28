#!/bin/bash
#
# Repository URL
# http://open-monica.googlecode.com/svn/trunk
#
# Poll SCM
# Every hour on the hour
# 0 * * * *

cd $WORKSPACE/trunk
ant
