from askapdev.rbuild.builders import Setuptools as Builder

builder = Builder()
builder.remote_archive = "mock-1.0.1.tar.gz"
builder.build()
