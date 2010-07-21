from askapdev.rbuild import setup
from askapdev.rbuild.dependencies import Dependency
from setuptools import find_packages
from setuptools.command.build_py import build_py
import glob
from icebuild import build_ice, clean_ice

class build_py2(build_py):
 
    def run(self):
        for cmd_name in self.get_sub_commands():
            print "DEBUG...."
            self.run_command(cmd_name)
 
        build_py.run(self)
     
build_py2.sub_commands.append(('build_ice', None))

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
      cmdclass = {'build_ice': build_ice, 
                  'build_py': build_py2,
                  'clean': clean_ice},
      options = { 'build_ice' : { 'interfaces' : slice_files,
                                  'dep' : dep ,
                                  'package' : 'askap.slice' }},
      #test_suite = "nose.collector",
)
