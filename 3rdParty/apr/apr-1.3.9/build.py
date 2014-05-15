import os
from askapdev.rbuild.builders import Autotools as Builder

builder = Builder()
builder.remote_archive = "apr-1.3.9.tar.gz"
builder.nowarnings = True
builder.add_option("--without-sendfile")
builder.build()
