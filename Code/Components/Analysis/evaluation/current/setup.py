from askapdev.rbuild.setup import setup
from askapdev.rbuild.dependencies import Dependency
from setuptools import find_packages

dep = Dependency()
dep.add_package()

ROOTPKG   = 'askap'
COMPONENT = 'analysis'
PKGNAME   = 'evaluation'

setup(name = '%s.%s.%s' % (ROOTPKG, COMPONENT, PKGNAME),
      version = 'current',
      description = 'Scripts to evaluate the results of the imaging and source finding',
      author = 'Matthew Whiting',
      author_email = 'Matthew.Whiting@csiro.au',
      url = 'http://svn.atnf.csiro.au/askap',
      keywords = ['ASKAP', 'Analysis'],
      long_description = '''\
These scripts take the results from the source-finding and look for any systematic effects 
such as positional offsets or flux errors compared to some reference catalogue. 
They then produce nifty plots that enable the user to quickly identify problems.
''',
      packages = find_packages(),
      namespace_packages = [ROOTPKG, '%s.%s' % (ROOTPKG, COMPONENT)],
      license = 'GPL',
      zip_safe = 1,
      dependency = dep,
# Uncomment if using scripts (applications which go in bin) 
      scripts = ["scripts/fluxEval.py","scripts/plotEval.py"],
# Uncomment if using unit tests
#      test_suite = "nose.collector",
)
