from askapdev.rbuild import setup
from askapdev.rbuild.dependencies import Dependency

dep = Dependency(silent=False)
dep.DEPFILE = "../dependencies"
dep.add_package()

setup(name = "logappender_test",
      dependency = dep,
      test_suite = "nose.collector",
)
