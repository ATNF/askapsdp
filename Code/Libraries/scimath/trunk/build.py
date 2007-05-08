# @file
# build script for AutoBuild
import sys
import os

if sys.argv.count("-q") > 0:
    redirect = " > /dev/null"
else:
    redirect = ""

os.system("scons %s" % redirect)
