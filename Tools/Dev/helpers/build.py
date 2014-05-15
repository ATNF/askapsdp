import os
import sys
from askapdev.rbuild.builders import Setuptools as Builder


# No docs here.
if 'doc' in sys.argv:
    sys.exit(0)

# Added to stop 'clean' target from warning about non-existent 'build/lib' dir.
def postcallback():
    target = os.path.join('build', 'lib')
    if not os.path.exists(target):
        os.makedirs(os.path.join('build', 'lib'))


builder = Builder('.')
builder.add_postcallback(postcallback)
builder.build()
