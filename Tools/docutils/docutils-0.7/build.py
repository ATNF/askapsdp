from askapdev.rbuild.builders import Setuptools as Builder

builder = Builder()
builder.remote_archive = "docutils-0.7.tar.gz"
builder.build()
