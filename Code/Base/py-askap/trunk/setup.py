from recursivebuild import setup
from recursivebuild.dependencies import Dependency
from setuptools import find_packages

dep = Dependency()
dep.add_package()

PKGNAME = ROOTPKG = "askap"

setup(name = PKGNAME,
      version = 'trunk',
      description = '<fill me in>',
      author = 'Joe Blog',
      author_email = 'Joe.Blog@example.com',
      url = 'http://svn.atnf.csiro.au/askap',
      keywords = ['ASKAP', '<more>',],
      long_description = '''Describe me...
''',
      packages = find_packages(),
#      namespace_packages = [ROOTPKG,ROOTPKG+"."+COMPONENT],
      license = 'GPL',
      zip_safe = 1,
      dependency = dep,
# Uncomment if using scripts (applications which go in bin) 
#      scripts = ["scripts/myapp.py"],
# Uncomment if using unit tests
#      test_suite = "test.suite",
)
