from askapdev.rbuild.builders import Ant as Builder

builder = Builder()
builder.remote_archive = "atoms-java-r5209.tar.gz"
builder.build()
