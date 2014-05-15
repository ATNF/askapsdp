from askapdev.rbuild.builders import Setuptools as Builder

builder = Builder()
builder.remote_archive = "scons-2.2.0.tar.gz"
builder.nowarnings = True
builder.build()
