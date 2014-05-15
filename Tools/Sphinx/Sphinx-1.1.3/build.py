from askapdev.rbuild.builders import Setuptools as Builder

builder = Builder()
builder.remote_archive = "Sphinx-1.1.3.tar.gz"
builder.build()
