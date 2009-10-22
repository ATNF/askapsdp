from askapdev.rbuild import setup
from askapdev.rbuild.dependencies import Dependency
from setuptools import find_packages

dep = Dependency()
dep.add_package()

ROOTPKG   = 'askap'
COMPONENT = 'analysis'
PKGNAME   = 'data'

setup(name = '%s.%s.%s' % (ROOTPKG, COMPONENT, PKGNAME),
      version = 'trunk',
      description = 'Scripts to create simulated data catalogues and images',
      author = 'MatthewWhiting',
      author_email = 'Matthew.Whiting@csiro.au',
      url = 'http://svn.atnf.csiro.au/askap',
      keywords = ['ASKAP', ],
      long_description = '''
This package contains some simulated data catalogues, 
as well as scripts to make more from the SKADS simulations. 
There are also scripts to make subsets of the catalogues and 
annotation file for use with the karma package.
''',
      packages = find_packages(),
      namespace_packages = [ROOTPKG, '%s.%s' % (ROOTPKG, COMPONENT)],
      license = 'GPL',
      zip_safe = 0,
      dependency = dep,
# Uncomment if using scripts (applications which go in bin) 
      scripts = ["scripts/createSKADS.py", 
                 "scripts/createSKADSspectralline.py", 
                 "scripts/createSubLists.py", 
                 "scripts/setupAllModels.py", 
                 "scripts/convolveModel.py",
                 "scripts/build_S3SAX_mySQLdb.py"],
# Uncomment if using unit tests
#      test_suite = "nose.collector",
)
