from askapdev.rbuild.builders import Autotools as Builder

builder = Builder()
builder.remote_archive = "gsl-1.10.tar.gz"
builder.nowarnings = True

builder.build()
