from askapdev.rbuild.builders import Setuptools as Builder

builder = Builder()
builder.remote_archive = "MySQL-python-1.2.4.tar.gz"

builder.nowarnings = True

builder.build()
