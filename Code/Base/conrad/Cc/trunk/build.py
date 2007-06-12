# @file
# build script for AutoBuild
import sys
import os
from recursivebuild import run

opts = ""
if "install" in sys.argv:
    opts = "install"

run("scons-0.96.95 %s" % opts )
