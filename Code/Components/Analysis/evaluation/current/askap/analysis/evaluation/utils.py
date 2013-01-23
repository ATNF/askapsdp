#!/usr/bin/env python

## @file
# A file containing utility functions to help with the plotting.

from pkg_resources import require
require('numpy')
require('matplotlib')
from pylab import *
from numpy import *
import gc

## @ingroup plotting
# @param array The array of values
def madfm(array):
    """
    Return the median absolute deviation from the median for an array of values
    """
    med = median(array)
    adfm = abs(array-med)
    return median(adfm)

## @ingroup plotting
# @param rms The rms value to be converted
def rmsToMADFM(rms=None):
    """ 
    Convert an rms value to a MADFM (median absolute deviation from
    median), assuming Gaussian statistics
    """
    return rms * 0.6744888

## @ingroup plotting
# @param mad The MADFM value to be converted
def madfmToRMS(mad=None):
    """ 
    Convert a MADFM (median absolute deviation from median) value to
    an rms, assuming Gaussian statistics
    """
    return mad / 0.6744888
