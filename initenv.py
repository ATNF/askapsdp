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
from distutils.sysconfig import get_python_inc

from optparse import OptionParser

parser = OptionParser()
parser.add_option("-s", "--shell",
                  dest="shell",
                  action="store", 
                  type="choice",
                  choices=["bash", "sh", "tcsh", "csh"],
                  default="bash",
                  help="specify the type of shell to generate the script for")

(opts, args) = parser.parse_args()

invoked_path = sys.argv[0]
absolute_path = os.path.abspath(invoked_path)
os.chdir(os.path.dirname(absolute_path))

bashinit = """\
ASKAP_ROOT=%s
export ASKAP_ROOT

pypath="${ASKAP_ROOT}/share/scons_tools"

if [ "${PYTHONPATH}" !=  "" ]
then
    PYTHONPATH=`echo $PYTHONPATH | sed "s#:*$pypath:*##"`
    PYTHONPATH="${pypath}:${PYTHONPATH}"
    PYTHONPATH=`echo ${PYTHONPATH} | sed -e '{s#^:##;s#:$##;}'`
else
    PYTHONPATH="${pypath}"
fi
export PYTHONPATH

PATH=`echo $PATH | sed "s#:*$ASKAP_ROOT/bin:*##"`
PATH="${ASKAP_ROOT}/bin:${PATH}"
export PATH

psset=`echo $PS1 | grep askap`
if [ "$psset" = "" ]
then
   PS1="(askap)${PS1}"
   export PS1
fi

MANPATH=`echo $MANPATH | sed "s#:*$ASKAP_ROOT/man:*##"`
MANPATH="${ASKAP_ROOT}/man:${MANPATH}"
export MANPATH

ARTISTIC_STYLE_OPTIONS="${ASKAP_ROOT}/astylerc"
export ARTISTIC_STYLE_OPTIONS

PYLINTRC="${ASKAP_ROOT}/pylintrc"
export PYLINTRC
""" % os.getcwd()

tcshinit = """\
setenv ASKAP_ROOT %s
set pypath="${ASKAP_ROOT}/share/scons_tools"

if ($?PYTHONPATH) then
    setenv PYTHONPATH `echo ${PYTHONPATH} | sed "s#:*${pypath}:*##"`
    setenv PYTHONPATH "${pypath}:${PYTHONPATH}"
    setenv PYTHONPATH `echo ${PYTHONPATH} | sed -e '{s#^:##;s#:$##;}'`
else
    setenv PYTHONPATH "${pypath}"
endif

setenv PATH `echo $PATH | sed "s#:*$ASKAP_ROOT/bin:*##"`
setenv PATH "${ASKAP_ROOT}/bin:${PATH}"


if ($?prompt) then
    set noglob
    set psset=`echo $prompt |grep askap`
    unset noglob
    if ("$psset" == "") then
        set prompt="\(askap\)${prompt}"
    endif
else
    set prompt="\(askap\) > "
endif

setenv MANPATH `echo $MANPATH | sed "s#:*$ASKAP_ROOT/man:*##"`
setenv MANPATH "${ASKAP_ROOT}/man:${MANPATH}"

setenv ARTISTIC_STYLE_OPTIONS "${ASKAP_ROOT}/astylerc"
setenv PYLINTRC "${ASKAP_ROOT}/Tools/Dev/rbuild/pylintrc"

"""  % os.getcwd()

shmap = {
        "bash" : { "suffix": "sh",  "init" : ".",      "file" : bashinit },
        "sh"   : { "suffix": "sh",  "init" : ".",      "file" : bashinit },
        "tcsh" : { "suffix": "csh", "init" : "source", "file" : tcshinit },
        "csh"  : { "suffix": "csh", "init" : "source", "file" : tcshinit },
        }

shell =  shmap[opts.shell]

filename = "initaskap.%s" % shell["suffix"]
if os.path.exists(filename):
    print "%s has already been generated. Remove it first to force regeneration." % filename
    sys.exit(0)
f = file(filename, "w")
f.write(shell["file"])
f.close()

pyvers = os.path.split(get_python_inc())[-1]

if not os.path.exists("include"):
    os.mkdir("include")
if not os.path.exists("include/%s" % pyvers):
    os.symlink(get_python_inc(), "include/%s" % pyvers)

print "Created initaskap.%s, please run '%s initaskap.%s' to initalise the environment" % ( shell["suffix"], shell["init"], shell["suffix"] )
