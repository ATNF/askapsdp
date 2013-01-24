#!/usr/bin/env python

## @file
# A file containing utility functions to help with the plotting.

from pkg_resources import require
require('numpy')
require('matplotlib')
from pylab import *
from numpy import *
import gc

def posToDec(pos):
    if(pos.find(':')>0):
        return dmsToDec(pos)
    else:
        return float(pos)

def dmsToDec(pos):
    bits=pos.split(':')
    return (abs(float(bits[0]))+float(bits[1])/60.+float(bits[2])/3600.)*sign(float(bits[0]))

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
