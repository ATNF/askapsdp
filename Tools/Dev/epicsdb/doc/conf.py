# DO THIS FIRST to set project name !!!
import askapdev.sphinx
# CAN NOT contain spaces!
askapdev.sphinx.project = u'askap.epicsdb'

from askapdev.sphinx.conf import *

#version = 'current'
#release = 'current'
extensions += ['askapdev.epicsdb.sphinxext']
html_sidebars = {'sphinx' : ['globaltoc.html', 'sourcelink.html', 'searchbox.html']}
