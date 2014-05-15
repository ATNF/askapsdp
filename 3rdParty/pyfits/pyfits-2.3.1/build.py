from askapdev.rbuild.builders import Setuptools as Builder

builder = Builder()
builder.remote_archive = "pyfits-2.3.1.tar.gz"
builder.nowarnings = True
builder.build()
