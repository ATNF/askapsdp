# @file
# build script for AutoBuild
import os
import sys

from askapdev.rbuild.builders import Scons as Builder

# Does not build on Debian etch 32 bit but should work on lenny.
# As yet do not have any 32 bit lenny machines.
# The conditional picks up all 

if sys.platform == 'linux2' and os.uname()[4] == 'i686':
    print "warn: Unable to build on etch 32 bit due to thread library confusion"
    sys.exit()

b = Builder(".")
b.build()

