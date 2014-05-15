import os

from askapdev.rbuild.builders import Autotools as Builder
import askapdev.rbuild.utils as utils

platform = utils.get_platform()
builder = Builder()
builder.remote_archive = "cppunit-1.12.0.tar.gz"

builder.add_option("--disable-doxygen")
builder.add_option("--disable-shared")
# See Issue #3942.  This is Ubuntu version 11 requirement.
# Ubuntu version 10 will work without it but does not hurt.
if platform['distribution'] == 'Ubuntu':
    builder.add_option("LIBS=-ldl")
builder.nowarnings = True

builder.build()
