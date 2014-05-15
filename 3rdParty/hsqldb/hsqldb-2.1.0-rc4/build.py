from askapdev.rbuild.builders import Data as Builder

builder = Builder()
builder.remote_archive = "hsqldb-2.1.0-rc4.tgz"
builder.build()
