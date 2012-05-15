import os
from askapdev.rbuild.dependencies import Dependency

# Ensure the state returned from getState() is as expected
def assertState(state):
    if cpadmin.getState() != state:
        raise RuntimeError("getState() returned incorrect state")

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
from askap.slice import CP

status = 0
ic = None
try:
    ic = Ice.initialize(sys.argv)
    base = ic.stringToProxy("CentralProcessorAdmin@CentralProcessorAdminAdapter")
    cpadmin = askap.interfaces.component.IComponentPrx.checkedCast(base)
    if not cpadmin:
        raise RuntimeError("Invalid proxy")

    # Startup
    config = dict()
    print "Calling Startup()...",
    cpadmin.startup(config)
    print "DONE"
    assertState(askap.interfaces.component.ComponentState.STANDBY)
    testresults = cpadmin.selfTest()
    print "Calling Activate()...",
    cpadmin.activate()
    print "DONE"
    assertState(askap.interfaces.component.ComponentState.ONLINE)

    # Execute a batch job
    servicebase = ic.stringToProxy("CentralProcessorService@CentralProcessorAdminAdapter")
    if not servicebase:
        raise RuntimeError("CentralProcessorService proxy not found")
    svc = askap.interfaces.cp.ICPObsServicePrx.checkedCast(servicebase)
    if not svc:
        raise RuntimeError("Invalid CPObsService proxy")

    print "Starting observation...",
    svc.startObs(0)
    print "DONE"

    print "Waiting for observation to complete...",
    svc.waitObs()
    print "DONE"

    # Shutdown
    print "Calling Deactivate()...",
    cpadmin.deactivate()
    print "DONE"
    assertState(askap.interfaces.component.ComponentState.STANDBY)
    print "Calling Shutdown()...",
    cpadmin.shutdown()
    print "DONE"

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
