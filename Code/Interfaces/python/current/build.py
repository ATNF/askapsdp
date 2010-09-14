import sys

from askapdev.rbuild.builders import Setuptools as Builder

# This generated package is not documented. The docs are build as part of ice.
if 'doc' in sys.argv:
    sys.exit(0)

builder = Builder(".")
builder.build()
