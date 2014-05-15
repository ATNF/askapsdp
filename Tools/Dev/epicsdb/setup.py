#
# This template file is defined for standalone scripts/modules
# that do not live in the askap namespace.  It is expected that
# these packages would only exist in Code/Systems/<system>
#
# These are simple scripts and modules NOT python packages.
# For the distinction, the "Distributing Python Modules" manual
# under Section 1.3 "General Python terminology" (pp.3 in Release 2.5.3).
# This is part of the normal Python documentation.

from askapdev.rbuild.setup import setup
from setuptools import find_packages

ROOTPKG = 'askapdev'
PKGNAME = 'epicsdb'

setup(name             = ".".join([ROOTPKG,PKGNAME]),
      description      = 'csv to EPICS DB Generator',
      author           = 'Craig Haskins',
      author_email     = 'Craig.Haskins@csiro.au',
      url              = 'http://pm.atnf.csiro.au/askap/projects/cmpt/wiki/DGS2EPICS_register_file',
      keywords         = ['ASKAP', 'EPICS'],
      long_description = '''Generate EPICS DB file from custom CSV format from DGS Group''',
      license          = 'GPL',
      zip_safe         = False,
      packages = find_packages(),
      namespace_packages = [ROOTPKG],
      #
      # Uncomment and edit the '###' lines as required.
      # Scripts will be installed in bin.
      scripts          = ["scripts/csv2epics.py", "scripts/adbe2epics.py"],
      # List any shared modules that the scripts wish to use here.
      # The names here must match files in modules subdirectory with
      # '.py' extension.
      ### py_modules       = ["common",]
      # Uncomment if using unit tests
      test_suite = "nose.collector",
     )
