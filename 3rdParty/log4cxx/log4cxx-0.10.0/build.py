import sys
import os
from askapdev.rbuild.builders import Autotools as Builder

#
# Fix for packages which depend on this package.
# They use -O2 which turns on -fstrict-aliasing and -Wall  and this whines with
#   warning: dereferencing type-punned pointer will break strict-aliasing rules 
# So explicitly turn it off.
# Cannot add as a postcallback as package.info not yet created.
#
def fix_package_info():
    found = False
    fp = open('package.info', 'r+')
    for line in fp:
        if line.startswith('defs='):
            found = True
            break
    if found != True:
        fp.write('defs=-fno-strict-aliasing')
    fp.close()

builder = Builder("apache-log4cxx-0.10.0")
builder.remote_archive = "apache-log4cxx-0.10.0.tar.gz"

apr     = builder.dep.get_install_path("apr")
aprutil = builder.dep.get_install_path("apr-util")

builder.add_option("--disable-doxygen")
builder.add_option("--with-apr=%s" % apr)
builder.add_option("--with-apr-util=%s" % aprutil)

if os.environ.has_key("CRAYOS_VERSION"):
    builder.add_option('LDFLAGS="-dynamic"')

builder.nowarnings = True

builder.build()

if 'install' in sys.argv[1:]:
    fix_package_info()
