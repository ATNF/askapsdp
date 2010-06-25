from askapdev.rbuild import setup
from askapdev.rbuild.dependencies import Dependency
from setuptools import find_packages
import glob

dep = Dependency()
dep.add_package()

slice_files = glob.glob('../../slice/current/*.ice')

ROOTPKG = "askap"
PKGNAME = "slice"

setup(name = "%s.%s" % (ROOTPKG, PKGNAME),
      version = 'current',
      description = 'ASKAP package for slice2py generated code.',
      author = 'Malte Marquarding',
      author_email = 'Malte.Marquarding@csiro.au',
      url = 'http://svn.atnf.csiro.au/askap',
      keywords = ['ASKAP', 'Interfaces', 'Ice'],
      long_description = '',
      packages = find_packages(),
      namespace_packages = [ROOTPKG],
      license = 'GPL',
      zip_safe = 0,
      dependency = dep,
      ice_interfaces = {"askap.slice": slice_files },
      #test_suite = "nose.collector",
)
