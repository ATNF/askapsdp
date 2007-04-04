## @file
# Script which creates a shell script that sets CONRAD specific environment
# variables. This is similar to AutoBuild bashrc, but intended for 
# developer use.
#
# copyright (c) 2007 CONRAD. All Rights Reserved.
# @author Malte Marquarding <Malte.Marquarding@csiro.au>
#
import sys
import os

filename = "initconrad.sh"
if os.path.exists(filename):
    sys.exit(0)
f = file(filename, "w")
exports = """
export CONRAD_PROJECT_ROOT=%s
export PYTHONPATH=${CONRAD_PROJECT_ROOT}/Tools/Dev/scons-tools
""" % os.getcwd()
f.write(exports)
f.close()
print "Created initconrad.sh, please run '. initconrad.sh' to initalise the environment"
