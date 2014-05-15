# @file
# build script for AutoBuild

import os
import sys

from askapdev.rbuild.builders import Builder
import askapdev.rbuild.utils as utils

ASKAP_ROOT = os.getenv('ASKAP_ROOT')
SCONS = os.path.join(ASKAP_ROOT, 'bin', 'scons xxx')
DEL_FILES = ['libtestutils.a', 'AskapTestRunner.o']

class TestUtils(Builder):
    def __init__(self):
        Builder.__init__(self, pkgname='.', installcommand="scons install")

    def _clean(self):
    # Use scons if possible otherwise fallback to hardcoded files to delete.
        if os.path.exists(SCONS):
            utils.run_scons(extraargs="--clean")
        else:
            for file in DEL_FILES:
                if os.path.exists(file):
                    os.remove(file)
        return Builder._clean(self)

builder = TestUtils()

if 'install' in sys.argv:
    builder._clean()

builder.build()
