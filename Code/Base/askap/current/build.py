# @file
# build script for AutoBuild

from askapdev.rbuild.builders import Scons as Builder

b = Builder(".")
b.add_install_file("files", "config")
b.build()
