# @file
# build script for AutoBuild

from recursivebuild.builders import Scons as Builder

b = Builder(".")
b.build()
