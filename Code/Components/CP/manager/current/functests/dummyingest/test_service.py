import os
from askapdev.rbuild.dependencies import Dependency

# main()
origdir = os.path.abspath(os.curdir)
os.chdir("../..")
dep = Dependency()
dep.add_package()
os.chdir(origdir)

import IcePy
import sys, traceback, Ice
import askap

# Import interfaces
from askap.slice import CommonTypes
from askap.slice import CP

status = 0
ic = None
try:
    ic = Ice.initialize(sys.argv)

    # Make a getServiceVersion() call on the service
    base = ic.stringToProxy("CentralProcessorService@CentralProcessorAdapter")
    if not base:
        raise RuntimeError("CentralProcessorService proxy not found")
    svc = askap.interfaces.cp.ICPObsServicePrx.checkedCast(base)
    if not svc:
        raise RuntimeError("Invalid IService proxy")
    print "Getting service version..."
    print "getServiceVersion() returned: " + svc.getServiceVersion()

    print "Starting observation...",
    svc.startObs(0)
    print "DONE"

    print "Aborting observation...",
    svc.abortObs()
    print "DONE"

    print "Waiting for observation to complete/abort...",
    svc.waitObs(-1)
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
