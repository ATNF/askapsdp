from askapdev.rbuild.builders import Setuptools as Builder

builder = Builder()
builder.remote_archive = "Jinja2-2.5.tar.gz"
builder.build()
