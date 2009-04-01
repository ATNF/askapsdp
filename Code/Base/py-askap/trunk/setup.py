from recursivebuild import setup
from recursivebuild.dependencies import Dependency
from setuptools import find_packages

dep = Dependency()
dep.add_package()

PKGNAME = ROOTPKG = "askap"
print find_packages()

setup(name = PKGNAME,
      version = 'trunk',
      description = 'ASKAP basic modules',
      author = 'Malte Marquarding',
      author_email = 'Malte.Marquarding@csiro.au',
      url = 'http://svn.atnf.csiro.au/askap',
      keywords = ['ASKAP', 'logging', 'Base'],
      long_description = '''General basic ASKAP modules such as logging
''',
      packages = find_packages(),
      license = 'GPL',
      zip_safe = 0,
      dependency = dep,
      scripts = ["scripts/logging_server.py"],
      package_data = {"": ["config/*.ice", 
                           "config/*.ice_cfg"] },
      # Uncomment if using unit tests
      #      test_suite = "test.suite",
)
