# @file
# build script for AutoBuild

from askapdev.rbuild.builders import Scons as Builder

b = Builder(".")

b.add_option("libtype=shared")

b.build()

