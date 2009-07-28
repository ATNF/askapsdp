# @file
# build script for AutoBuild

import sys

from askapdev.rbuild.builders import Scons as Builder
import askapdev.rbuild.utils as utils

if not utils.which("mpicxx"):
    print "warn: Cannot build package as no MPI support found on this platform."
    sys.exit(0)

b = Builder(".")
b.build()
