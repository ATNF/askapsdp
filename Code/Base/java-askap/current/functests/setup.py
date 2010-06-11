from askapdev.rbuild import setup
from askapdev.rbuild.dependencies import Dependency

dep = Dependency(silent=False)
dep.DEPFILE = "../dependencies"
dep.add_package()

setup(name = "javalogger_test",
      dependency = dep,
      ice_interfaces = {".": ["CommonTypes.ice",
                              "LoggingService.ice",
                              "TypedValues.ice"] },
      test_suite = "nose.collector",
)
