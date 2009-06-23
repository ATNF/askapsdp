from askapdev.rbuild import setup
from askapdev.rbuild.dependencies import Dependency
from setuptools import find_packages

dep = Dependency()
dep.add_package()

ROOTPKG   = 'askap'
COMPONENT = 'opl'
PKGNAME   = 'ptfopl'

setup(name = '%s.%s.%s' % (ROOTPKG, COMPONENT, PKGNAME),
      version = 'trunk',
      description = 'Observation Procedure Library for Parkes Testbed Facility',
      author = 'JuanCarlosGuzman',
      author_email = 'Juan.Guzman@csiro.au',
      url = 'http://svn.atnf.csiro.au/askap',
      keywords = ['ASKAP', 'opl', 'ptf'],
      long_description = '''Observation Procedure Library for PTF''',
      packages = find_packages(),
      namespace_packages = [ROOTPKG, '%s.%s' % (ROOTPKG, COMPONENT)],
      license = 'GPL',
      zip_safe = 1,
      dependency = dep,
# Uncomment if using scripts (applications which go in bin) 
#      scripts = ["scripts/myapp.py"],
# Uncomment if using unit tests
#      test_suite = "nose.collector",
)
