from askapdev.rbuild.builders import Ant as Builder

builder = Builder(".")
builder.add_install_file("files", "config")
builder.build()
