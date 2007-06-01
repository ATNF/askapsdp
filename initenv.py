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
		     "init" : "."},
	  "tcsh" : { "suffix": "csh", "envset": "setenv", 
		     "envassign": " ",
		     "init" : "source"} }


shell =  shmap[opts.shell]

filename = "initconrad.%s" % shell["suffix"]
if os.path.exists(filename):
    sys.exit(0)
f = file(filename, "w")
exports = "%s CONRAD_PROJECT_ROOT%s%s\n" % ( shell["envset"], 
					     shell["envassign"],
					     os.getcwd() )
exports += "%s PYTHONPATH%s${CONRAD_PROJECT_ROOT}/lib/python:${CONRAD_PROJECT_ROOT}/Tools/Dev/scons-tools\n" % (shell["envset"], shell["envassign"])
exports += "%s PATH%s${CONRAD_PROJECT_ROOT}/bin:${PATH}" % (shell["envset"], 
							    shell["envassign"])
f.write(exports)
f.close()

if not os.path.exists("lib"):
    os.mkdir("lib")
    os.mkdir("lib/python")
if not os.path.exists("bin"):
    os.mkdir("bin")
dconf = os.path.expanduser("~/.pydistutils.cfg")

if os.path.exists(dconf):
    print "Moving existing ~/.pydistutils.cfg out of the way"
    os.rename(dconf, dconf+".save")
    print """Warning this changes the default location of the 
installation of python packages to the CONRAD tree."""

f = file(dconf, "w")
f.write("""
[install]
install_lib = $CONRAD_PROJECT_ROOT/lib/python
install_scripts = $CONRAD_PROJECT_ROOT/bin
""")
f.close()

print "Created initconrad.%s, please run '%s initconrad.%s' to initalise the environment" % ( shell["suffix"], shell["init"], shell["suffix"] )
