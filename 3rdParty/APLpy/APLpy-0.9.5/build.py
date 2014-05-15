from askapdev.rbuild.builders import Setuptools as Builder

builder = Builder()
builder.remote_archive = "APLpy-0.9.5.tar.gz"
builder.build()
