# DO THIS FIRST to set project name !!!                                                                 
import askapdev.sphinx
# CAN NOT contain spaces!                                                                               
askapdev.sphinx.project = u'CentralProcessor'

from askapdev.sphinx.conf import *
import subprocess

def svnversion():
    p = subprocess.Popen("svnversion", shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    (stdout, stderr) = p.communicate()
    return stdout

# For a "release" uncomment these and set the release name
#version = '0.1'
#release = '0.1-draft'

# For a snapshot use the svn revision instead
version = 'r' + svnversion()
release = version
