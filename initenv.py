## @file
# Script which creates a shell script that sets CONRAD specific environment
# variables. This is similar to AutoBuild bashrc, but intended for 
# developer use.
#
# copyright (c) 2007 CONRAD. All Rights Reserved.
# @author Malte Marquarding <Malte.Marquarding@csiro.au>
#
import sys
import os
from distutils.sysconfig import get_python_version

from optparse import OptionParser

parser = OptionParser()
parser.add_option("-s", "--shell",
                  dest="shell",
                  action="store", 
                  type="choice",
		  choices=["bash", "tcsh"],
                  default="bash",
                  help="specify the type of shell to generate the script for")

(opts, args) = parser.parse_args()

pyvers = "python%s" % get_python_version()
pylibdir = "lib/%s" % pyvers

bashinit = """\
export CONRAD_PROJECT_ROOT=%s
pypath="${CONRAD_PROJECT_ROOT}/lib/%s:${CONRAD_PROJECT_ROOT}/Tools/Dev/scons-tools"

if [ "${PYTHONPATH}" !=  "" ]
then
    PYTHONPATH=`echo $PYTHONPATH | sed "s#:*$pypath:*##"`
    PYTHONPATH="${pypath}:${PYTHONPATH}"
    PYTHONPATH=`echo ${PYTHONPATH} | sed -e '{s#^:##;s#:$##;}'`
else
    PYTHONPATH="${pypath}"
fi
export PYTHONPATH

PATH=`echo $PATH | sed "s#:*$CONRAD_PROJECT_ROOT/bin:*##"`
PATH="${CONRAD_PROJECT_ROOT}/bin:${PATH}"
export PATH

psset=`echo $PS1 |grep askap`
if [ "$psset" == "" ]
then
   export PS1="(askap)${PS1}"
fi

MANPATH=`echo $MANPATH | sed "s#:*$CONRAD_PROJECT_ROOT/man:*##"`
MANPATH="${CONRAD_PROJECT_ROOT}/man:${MANPATH}"
export MANPATH

""" % (os.getcwd(), pyvers)

tcshinit = """\
setenv CONRAD_PROJECT_ROOT %s
set pypath="${CONRAD_PROJECT_ROOT}/lib/%s:${CONRAD_PROJECT_ROOT}/Tools/Dev/scons-tools"

if ($?PYTHONPATH) then
    setenv PYTHONPATH `echo ${PYTHONPATH} | sed "s#:*${pypath}:*##"`
    setenv PYTHONPATH "${pypath}:${PYTHONPATH}"
    setenv PYTHONPATH `echo ${PYTHONPATH} | sed -e '{s#^:##;s#:$##;}'`
else
    setenv PYTHONPATH "${pypath}"
endif

setenv PATH `echo $PATH | sed "s#:*$CONRAD_PROJECT_ROOT/bin:*##"`
setenv PATH "${CONRAD_PROJECT_ROOT}/bin:${PATH}"


set psset=`echo $prompt |grep conrad`
if ("$psset" == "") then
   set prompt="\(conrad\)${prompt}"
endif

setenv MANPATH `echo $MANPATH | sed "s#:*$CONRAD_PROJECT_ROOT/man:*##"`
setenv MANPATH "${CONRAD_PROJECT_ROOT}/man:${MANPATH}"
"""  % (os.getcwd(), pyvers)


shmap = { "bash" : { "suffix": "sh",
		     "init" : ".", "file" : bashinit
                     },
	  "tcsh" : { "suffix": "csh",
		     "init" : "source", "file" : tcshinit
                     }
          }


shell =  shmap[opts.shell]

filename = "initaskap.%s" % shell["suffix"]
if os.path.exists(filename):
    print "%s has already been generated. Remove it first to force regeneration." % filename
    sys.exit(0)
f = file(filename, "w")
f.write(shell["file"])
f.close()

if not os.path.exists(pylibdir):
    os.makedirs(pylibdir)
if not os.path.exists("bin"):
    os.mkdir("bin")

print "Created initaskap.%s, please run '%s initaskap.%s' to initalise the environment" % ( shell["suffix"], shell["init"], shell["suffix"] )
