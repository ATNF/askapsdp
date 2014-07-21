from askapdev.rbuild.builders import Setuptools as Builder

def dummy():
    pass

builder = Builder(".")
builder._install = dummy
builder.build()
