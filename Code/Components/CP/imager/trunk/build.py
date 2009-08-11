# @file
# build script for AutoBuild

from askapdev.rbuild.builders import Scons as Builder
import askapdev.rbuild.utils as utils

b = Builder(".")
b.build()
