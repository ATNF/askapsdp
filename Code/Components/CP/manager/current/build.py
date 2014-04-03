import subprocess
import string
import sys
from askapdev.rbuild.builders import Ant as Builder

##########################################################################
# This is a temporary check for Java 7. Once we deprecate our Java 6 only
# platforms (Debian Squeeze , OSX Snow Leopard) this check can be removed
def is_java7():
    sp = subprocess.Popen(["java", "-version"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out = sp.communicate()
    sp.wait()
    idx = out[1].find('1.7')
    return idx != -1

if not is_java7():
    print "Note: No Java 7 present, skipping build"
    sys.exit(0)
##########################################################################

builder = Builder(".")
builder.add_run_script("cpmanager.sh", "askap/cp/manager/CpManager")

builder.build()
