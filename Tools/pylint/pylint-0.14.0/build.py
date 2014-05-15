from askapdev.rbuild.builders import Setuptools as Builder

builder = Builder()
builder.remote_archive = "pylint-0.14.0.tar.gz"
builder.add_file("setupegg.py")
builder.nowarnings = True
builder.build()
