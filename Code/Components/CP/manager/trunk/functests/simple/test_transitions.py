import os
from pkg_resources import require
from askapdev.rbuild.dependencies import Dependency

def transitions():
    config = dict()
    print "Calling Startup...",
    cpadmin.startup(config)
    print "DONE"
    print "Calling Activate...",
    cpadmin.activate()
    print "DONE"
    print "Calling Deactivate...",
    cpadmin.deactivate()
    print "DONE"
    print "Calling Activate...",
    cpadmin.activate()
    print "DONE"
    print "Calling Deactivate...",
    cpadmin.deactivate()
    print "DONE"
    print "Calling Shutdown...",
    cpadmin.shutdown()
    print "DONE"

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
    if not cpadmin:
        raise RuntimeError("Invalid proxy")

    # Run through the state transitions
    for n in range(0, 4):
        transitions()
        time.sleep(1)
except:
    traceback.print_exc()
    status = 1
finally:
    try:
        if ic is not None:
            ic.destroy()
    except:
        traceback.print_exc()
        status = 1
sys.exit(status)
