import os
from askapdev.rbuild.builders import Autotools as Builder

builder = Builder()
builder.remote_archive = "xerces-c-3.1.1.tar.gz"

if os.environ.has_key("CRAYOS_VERSION"):
    builder.add_option('LDFLAGS="-dynamic"')

builder.build()
