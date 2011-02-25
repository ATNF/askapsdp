import os
from pkg_resources import require
from askapdev.rbuild.dependencies import Dependency

# Check the service interface is available
def checkService():
    servicebase = ic.stringToProxy("SkyModelService")
    if not servicebase:
        raise RuntimeError("SkyModelService proxy not found")

# Ensure the state returned from getState() is as expected
def assertState(state):
    if cpadmin.getState() != state:
        raise RuntimeError("getState() returned incorrect state")

# Run through the transitions
def transitions():
    config = dict()
    print "Calling Startup()...",
    cpadmin.startup(config)
    print "DONE"
    assertState(askap.interfaces.component.ComponentState.STANDBY)
    testresults = cpadmin.selfTest()
    for n in range(0, 2):
        print "  Calling Activate()...",
        cpadmin.activate()
        print "DONE"
        assertState(askap.interfaces.component.ComponentState.ONLINE)
        checkService()
        print "  Calling Deactivate()...",
        cpadmin.deactivate()
        print "DONE"
        assertState(askap.interfaces.component.ComponentState.STANDBY)
    print "Calling Shutdown()...",
    cpadmin.shutdown()
    print "DONE"

# main()
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

# Import interfaces
import CommonTypes_ice
import Component_ice

status = 0
ic = None
try:
    ic = Ice.initialize(sys.argv)
    base = ic.stringToProxy("SkyModelServiceAdmin")
    cpadmin = askap.interfaces.component.IComponentPrx.checkedCast(base)
    if not cpadmin:
        raise RuntimeError("Invalid proxy")

    # Run through the state transitions
    for n in range(0, 4):
        transitions()
        time.sleep(0.5)
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
