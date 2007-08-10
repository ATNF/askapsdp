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

shmap = { "bash" : { "suffix": "sh", "envset": "export", 
		     "envassign": "=",
		     "init" : ".", 
                     "prompt" : 'export PS1="(conrad)${PS1}"'},
	  "tcsh" : { "suffix": "csh", "envset": "setenv", 
		     "envassign": " ",
		     "init" : "source",
                     "prompt" : 'set prompt="\(conrad\)${prompt}"'} }


shell =  shmap[opts.shell]
pylibdir = "lib/python%s" % get_python_version()

filename = "initconrad.%s" % shell["suffix"]
if os.path.exists(filename):
    print "%s has already been generated. Remove it first to force regeneration." % filename
    sys.exit(0)
f = file(filename, "w")
exports = "%s CONRAD_PROJECT_ROOT%s%s\n" % ( shell["envset"], 
					     shell["envassign"],
					     os.getcwd() )
exports += "%s PYTHONPATH%s${CONRAD_PROJECT_ROOT}/%s:${CONRAD_PROJECT_ROOT}/Tools/Dev/scons-tools\n" % (shell["envset"], shell["envassign"], pylibdir)
exports += "%s PATH%s${CONRAD_PROJECT_ROOT}/bin:${PATH}\n" % (shell["envset"], 
                                                              shell["envassign"])

exports += shell["prompt"]+"\n"
   
exports += '%s MANPATH%s${CONRAD_PROJECT_ROOT}/man:${MANPATH}\n' % (shell["envset"],shell["envassign"])
f.write(exports)
f.close()

if not os.path.exists(pylibdir):
    os.makedirs(pylibdir)
if not os.path.exists("bin"):
    os.mkdir("bin")

print "Created initconrad.%s, please run '%s initconrad.%s' to initalise the environment" % ( shell["suffix"], shell["init"], shell["suffix"] )
