## @file
#  Wrapper around the standard logging package and the XMLLayout module for 
#  adding a XMLSocketAppender sending log4j xml formatted eevents
#
# copyright (c) 2007 ASKAP. All Rights Reserved.
# @author Malte Marquarding <malte.marquarding@csiro.au>
#

# make this module look like logging
# since we know what we are doing allow wildcard
# pylint: disable-msg=W0401

from logging import *
from xmllayout import RawSocketHandler as XMLSocketHandler



