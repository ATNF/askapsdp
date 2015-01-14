from askapdev.rbuild.builders import Setuptools as Builder

builder = Builder()
builder.remote_archive = "scons-2.3.4.tar.gz"
builder.nowarnings = True
builder.build()
