from askapdev.rbuild.builders import Scons as Builder

builder = Builder()
builder.remote_archive = "cmdlineparser-0.1.1.tar.bz2"
builder.build()
