# DO THIS FIRST to set project name !!!                                                                 
import askapdev.sphinx
# CAN NOT contain spaces!                                                                               
askapdev.sphinx.project = u'CentralProcessorAdministrator'

from askapdev.sphinx.conf import *
import subprocess

def svnversion():
    p = subprocess.Popen("svnversion", shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    (stdout, stderr) = p.communicate()
    return stdout

# For a "release" uncomment these and set the release name
version = '0.3.0'
release = '0.3.0'

# For a snapshot use the svn revision instead
#version = 'r' + svnversion()
#release = version
