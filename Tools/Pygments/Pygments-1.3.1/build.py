from askapdev.rbuild.builders import Setuptools as Builder

builder = Builder()
builder.remote_archive = "Pygments-1.3.1.tar.gz"
builder.build()
