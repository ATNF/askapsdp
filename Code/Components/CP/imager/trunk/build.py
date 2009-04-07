# @file
# build script for AutoBuild

from recursivebuild.thirdpartybuilder import SconsBuilder as Builder

b = Builder(".")
b.build()
