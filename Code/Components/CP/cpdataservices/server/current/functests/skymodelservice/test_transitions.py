import os
from askapdev.rbuild.dependencies import Dependency

# Check the service interface is available
def checkService():
    servicebase = ic.stringToProxy("SkyModelService@SkyModelServiceAdminAdapter")
    if not servicebase:
        raise RuntimeError("SkyModelService proxy not found")

# Ensure the state returned from getState() is as expected
def assertState(state):
    if smsadmin.getState() != state:
        raise RuntimeError("getState() returned incorrect state")

# Run through the transitions
def transitions():
    config = dict()
    print "Calling Startup()...",
    smsadmin.startup(config)
    print "DONE"
    assertState(askap.interfaces.component.ComponentState.STANDBY)
    testresults = smsadmin.selfTest()
    for n in range(0, 2):
        print "  Calling Activate()...",
        smsadmin.activate()
        print "DONE"
        assertState(askap.interfaces.component.ComponentState.ONLINE)
        checkService()
        print "  Calling Deactivate()...",
        smsadmin.deactivate()
        print "DONE"
        assertState(askap.interfaces.component.ComponentState.STANDBY)
    print "Calling Shutdown()...",
    smsadmin.shutdown()
    print "DONE"

# main()
origdir = os.path.abspath(os.curdir)
os.chdir("../..")
dep = Dependency()
dep.add_package()
os.chdir(origdir)

import IcePy
import time
import sys, traceback, Ice
import askap

# Import interfaces
from askap.slice import CommonTypes
from askap.slice import Component

status = 0
ic = None
try:
    ic = Ice.initialize(sys.argv)
    base = ic.stringToProxy("SkyModelServiceAdmin@SkyModelServiceAdminAdapter")
    smsadmin = askap.interfaces.component.IComponentPrx.checkedCast(base)
    if not smsadmin:
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
