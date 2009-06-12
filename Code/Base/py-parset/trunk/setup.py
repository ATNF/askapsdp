from askapdev.rbuild import setup
from askapdev.rbuild.dependencies import Dependency
from setuptools import find_packages

dep = Dependency()
dep.add_package()

PKGNAME = "parset"
ROOTPKG = "askap"

setup(name = '%s.%s' % (ROOTPKG, PKGNAME),
      version = 'trunk',
      description = 'LOFAR ParameterSet parser',
      author = 'Malte Marquarding',
      author_email = 'Malte.Marquarding@csiro.au',
      url = 'http://svn.atnf.csiro.au/askap',
      keywords = ['ASKAP', 'ParameterSet', 'Base', 'LOFAR'],
      long_description = '''This package provides native python parsing of LOFAR ParamterSets.
''',
      packages = find_packages(),
      namespace_packages = [ROOTPKG],
      license = 'GPL',
      zip_safe = 1,
      dependency = dep,
      test_suite = "nose.collector",
)
