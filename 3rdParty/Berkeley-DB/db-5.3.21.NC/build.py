import os

from askapdev.rbuild.builders import Autotools as Builder
import askapdev.rbuild.utils as utils

builder = Builder(buildsubdir="build_unix",
                  confcommand='../dist/configure')
builder.remote_archive = "db-5.3.21.NC.tar.gz"

if os.uname()[4] == 'x86_64':
    builder.add_option('--with-pic')

#builder.add_option('--disable-shared') # Need shared libraries for Java.
builder.add_option('--enable-cxx')
builder.add_option('--enable-java')
builder.nowarnings = True

# On Mac OSX jni.h is in a  location where BerkleyDB can't find it. Including
# $JAVA_HOME/include (and include/darwin) fixes this. The JAVA_HOME environment
# can be setup on OSX like so (for bash): export JAVA_HOME=$(/usr/libexec/java_home)
platform =  utils.get_platform()
if platform['system'] == 'Darwin' and os.environ.has_key("JAVA_HOME"):
    javahome = os.environ.get('JAVA_HOME')
    builder.add_option('CPPFLAGS="-I%s/include -I%s/include/darwin"' %(javahome,javahome))

# The Cray cc and c++ compilers wrappers break here, so go directly to gcc and g++
if os.environ.has_key("CRAYOS_VERSION"):
    builder.add_env("CC","gcc")
    builder.add_env("CXX","g++")
    builder.add_env("LINK","g++")
    builder.add_env("SHLINK","g++")

builder.build()
