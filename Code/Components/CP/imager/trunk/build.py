# @file
# build script for AutoBuild

import os
import sys

from askapdev.rbuild.builders import Scons as Builder
from askapdev.rbuild.dependencies import Dependency
import askapdev.rbuild.utils as utils

# Openmpi is virtual package, and want to know if the symlink has been
# created.
dep = Dependency(silent=True)
dep.add_package()
openmpi = dep.get_install_path("openmpi")

if not os.path.exists(openmpi):
    print "warn: Cannot build package as no MPI support found on this platform."
    sys.exit(0)

b = Builder(".")
b.build()
