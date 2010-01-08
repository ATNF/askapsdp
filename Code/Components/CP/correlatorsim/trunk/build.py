# @file
# build script for AutoBuild
import os
import platform
import sys

from askapdev.rbuild.builders import Scons as Builder

# Does not build on Debian etch 32 bit but should work on lenny.
# As yet do not have any 32 bit lenny machines.

dist, ver = platform.dist()[0:2]
arch = platform.architecture()[0]

if dist == 'debian' and ver.startswith('4.') and arch == '32bit':
    print "warn: Unable to build on etch 32 bit due to thread library confusion"
    sys.exit()

b = Builder(".")
b.build()

