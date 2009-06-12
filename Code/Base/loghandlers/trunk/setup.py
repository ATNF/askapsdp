from askapdev.rbuild import setup
from askapdev.rbuild.dependencies import Dependency
from setuptools import find_packages

dep = Dependency()
dep.add_package()

ROOTPKG = "askap"
PKGNAME = "loghandlers"

setup(name = "%s.%s" % (ROOTPKG, PKGNAME),
      version = 'trunk',
      description = 'ASKAP logging handler extensions',
      author = 'Malte Marquarding',
      author_email = 'Malte.Marquarding@csiro.au',
      url = 'http://svn.atnf.csiro.au/askap',
      keywords = ['ASKAP', 'logging', 'Base', 'Ice'],
      long_description = '''Adddional logginh handler for python. This includes and ice handler.''',
      packages = find_packages(),
      namespace_packages = [ROOTPKG],
      license = 'GPL',
      zip_safe = 0,
      dependency = dep,
      scripts = ["scripts/logging_server.py"],
      package_data = {"": ["config/*.ice", 
                           "config/*.ice_cfg"] },
      ice_interfaces = {"askap.loghandlers": ["LoggingService.ice"] },
      test_suite = "nose.collector",
)
