# @file
# build script for AutoBuild

from askapdev.rbuild.thirdpartybuilder import SconsBuilder as Builder

b = Builder(".")
b.build()

