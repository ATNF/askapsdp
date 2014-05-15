from askapdev.rbuild.builders import Autotools as Builder

builder = Builder()
builder.remote_archive = "cmake-2.8.12.2.tar.gz"
builder.nowarnings = True
builder.build()
