import sys
import os

from askapdev.rbuild.builders import Virtual as Builder
import askapdev.rbuild.utils as utils

exe = 'mysql_config'

syspath = utils.which(exe)

#ignore system path
if syspath and syspath.startswith('/usr/bin'):
    sys.exit(0)

# OS X specific
os.environ["PATH"] = os.path.pathsep.join(["/usr/local/mysql/bin",#OS X dmg
                                           "/opt/local/bin", # macports
                                           os.environ["PATH"]])

builder = Builder(pkgname='mysql', exename=exe)
builder.build()
