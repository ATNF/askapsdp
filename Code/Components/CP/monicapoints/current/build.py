import os

from askapdev.rbuild.builders import Builder

builder = Builder('.')

builder.add_install_file("files")
builder.build()
