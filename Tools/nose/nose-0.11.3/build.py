from askapdev.rbuild.builders import Setuptools as Builder

builder = Builder()
builder.remote_archive = "nose-0.11.3.tar.gz"
builder.nowarnings = True
builder.build()
