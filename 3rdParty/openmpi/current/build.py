import os
from askapdev.rbuild.builders import Virtual as Builder
import askapdev.rbuild.utils as utils

# Need to specify module name explicitly unless other builders
if not os.environ.has_key("CRAYOS_VERSION"):
    builder = Builder(pkgname='openmpi', exename="mpicxx")
    builder.build()
