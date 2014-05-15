import os
import distutils.sysconfig

from askapdev.rbuild.builders import Autotools as Builder
from askapdev.rbuild.builders import Setuptools as Builder2
import askapdev.rbuild.utils as utils

targets = 'cpp py java'

os.environ['PYTHON_HOME'] = os.getenv('ASKAP_ROOT')

# Create the builder now as we need builder._prefix.
builder = Builder(confcommand=None)
builder.remote_archive = "Ice-3.5.0.tar.gz"

db      = builder.dep.get_install_path("db")
bzip2   = builder.dep.get_install_path("bzip2")
expat   = builder.dep.get_install_path("expat")
mcpp    = builder.dep.get_install_path("mcpp")
openssl = builder.dep.get_install_path("openssl")

# Required for java build
oldldp = os.environ.get('LD_LIBRARY_PATH', '')
newldp = os.path.join(db, "lib")
oldcp  = os.environ.get('CLASSPATH', '')
newcp  = os.path.join(db, "lib", "db.jar")

builder.add_classpath_paths(newcp)
builder.add_ld_library_paths(newldp)

top_m   = [('^SUBDIRS.*=.*',         'SUBDIRS  = %s' % targets),
           ('^INSTALL_SUBDIRS.*=.*', 'INSTALL_SUBDIRS  = %s' % targets)]
cpp_mr  = [('^prefix.*?=.*',         'prefix = %s' % builder._prefix),
           ('^#DB_HOME.*',           'DB_HOME = %s' % db),
           ('^#BZIP2_HOME.*',        'BZIP2_HOME = %s' % bzip2),
           ('^#EXPAT_HOME.*',        'EXPAT_HOME = %s' % expat),
           ('^#MCPP_HOME.*',         'MCPP_HOME = %s' % mcpp),
           ('^#OPENSSL_HOME.*',      'OPENSSL_HOME = %s' % openssl),
           ('^(embedded_runpath_prefix.*?=.*)',   '#\\1')]

pyver = 'python%s' % distutils.sysconfig.get_python_version()
pyincdir = os.path.join(os.getenv('ASKAP_ROOT'), 'include', pyver)
pylibdir = distutils.sysconfig.get_config_var('LIBDIR')
py_mr   = [('^prefix.*?=.*',         'prefix = %s' % builder._prefix),
           ('^PYTHON_VERSION.*?=.*', 'PYTHON_VERSION = %s' % pyver),
           ('^PYTHON_INCLUDE_DIR.*?=.*', 'PYTHON_INCLUDE_DIR = %s' % pyincdir),
           ('^PYTHON_LIB_DIR.*?=.*', 'PYTHON_LIB_DIR = %s' % pylibdir),
           ('^(embedded_runpath_prefix.*?=.*)', '#\\1')]
py_mrd  = [('(\$\(PYTHON_HOME\)/)Python' , '\\1.Python'),
           ('(PYTHON_LIBS.*=).*',     '\\1 $(PYTHON_HOME)/.Python')]

java_bp = [('^#prefix.*?=.*/opt.*',   'prefix = %s' % builder._prefix)]

java_ant =  [('(Ice)(\.jar)',         '\\1-${ice.version}\\2'),
             ('(ant-ice)(\.jar)',     '\\1-${ice.version}\\2')]


repdict = {'Makefile'                    : top_m,
           'cpp/config/Make.rules'       : cpp_mr,
           'py/config/Make.rules'        : py_mr,
           'py/config/Make.rules.Darwin' : py_mrd,
           'java/config/build.properties': java_bp,
           'java/build.xml': java_ant}

platform = utils.get_platform()

for filename in repdict.keys():
    for r_tuple in repdict[filename]:
        builder.replace(filename, *r_tuple) # '*' operator - unpack the tuple.

builder.parallel = False
builder.nowarnings = True

builder.build()


# The first build installs python just as files in directory 'python'.
# We want it installed as egg.
utils.rmtree(os.path.join(builder._prefix, "python"))

builder2 = Builder2(buildsubdir='py')

builder2.add_file('files/setup.py', 'py')
builder2.add_file('files/setup.cfg', 'py')
builder2.do_clean = False

builder2.build()
