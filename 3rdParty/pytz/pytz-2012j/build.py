from askapdev.rbuild.builders import Setuptools as Builder

builder = Builder()
builder.remote_archive = "pytz-2012j.tar.gz"
builder.build()
