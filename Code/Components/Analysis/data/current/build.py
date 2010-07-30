import sys

from askapdev.rbuild.builders import Setuptools as Builder

if 'doc' in sys.argv:
    print 'warn: unable to handle doc target'
    sys.exit(0)

builder = Builder(".")
builder.build()
