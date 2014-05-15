# Non standard install.
# In scons, tools must live in a directory and be specified by a file system
# path.  They cannot be in a python package.
# We want to install into the top level ASKAPsoft tree (like all other tools)
# and not hard coded to a location in the Tools subdirectory.

import glob
import shutil
import sys
import os

from askapdev.rbuild.builders import Builder

class myBuilder(Builder):
    def __init__(self):
        Builder.__init__(self, pkgname='.', archivename=None, extractdir=None)

    def _build(self):
        pass

    def _install(self):
        files = glob.glob("*.py")
        ASKAP_ROOT = os.environ['ASKAP_ROOT']
        ANT_TOOLS_DIR = os.path.join(ASKAP_ROOT, 'share', 'ant_tools')

        if not os.path.exists(ANT_TOOLS_DIR):
            os.makedirs(ANT_TOOLS_DIR)

        for file in glob.glob('*.xml'):
            shutil.copy(file, ANT_TOOLS_DIR)

builder = myBuilder()
builder.build()
