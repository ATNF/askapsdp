import os
from pkg_resources import require
from askapdev.rbuild.dependencies import Dependency

origdir = os.path.abspath(os.curdir)
os.chdir("../..")
dep = Dependency()
dep.add_package()
os.chdir(origdir)

require(dep.get_python_requires())
import IcePy
import time
import sys, traceback, Ice
import askap
import CommonTypes_ice
import Component_ice

status = 0
ic = None
try:
    ic = Ice.initialize(sys.argv)
    base = ic.stringToProxy("CentralProcessorAdmin")
    cpadmin = askap.interfaces.component.IComponentPrx.checkedCast(base)
    config = dict()
    print "Calling Startup..."
    cpadmin.startup(config)
    time.sleep(1)
    print "Calling Activate..."
    cpadmin.activate()
    time.sleep(1)
    print "Calling Deactivate..."
    cpadmin.deactivate()
    time.sleep(1)
    print "Calling Shutdown..."
    cpadmin.shutdown()
    time.sleep(1)
    print "DONE"
    if not cpadmin:
        raise RuntimeError("Invalid proxy")
except:
    traceback.print_exc()
    status = 1


# Clean up
if ic:
    try:
        ic.destroy()
    except:
        traceback.print_exc()
        status = 1
        sys.exit(status)
