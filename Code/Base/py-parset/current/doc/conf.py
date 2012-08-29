# DO THIS FIRST to set project name !!!
from askapdev.sphinx import project
# CAN NOT contain spaces!
project = u'askap.parset'

from askapdev.sphinx.conf import *

version = 'current'
release = 'current'
extensions += ['askap.parset.sphinxext']
