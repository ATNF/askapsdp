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
from askap.slice import IService

status = 0
ic = None
try:
    ic = Ice.initialize(sys.argv)

    # Make a getServiceVersion() call on the service
    base = ic.stringToProxy("SkyModelService@SkyModelServiceAdapter")
    if not base:
        raise RuntimeError("SkyModelService proxy not found")
    svc = askap.interfaces.services.IServicePrx.checkedCast(base)
    if not svc:
        raise RuntimeError("Invalid IService proxy")
    print "getServiceVersion() returned: " + svc.getServiceVersion()

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
