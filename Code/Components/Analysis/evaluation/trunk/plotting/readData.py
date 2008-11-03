#!/usr/bin/env python

## @file
#  A file containing functions to read in data ready for plotting. It
#  can read data for matching sources and sources that didn't match.

## @namespace readData
#  Data I/O for python scripts.
#  A set of functions to read in the results of the pattern matching,
#  so that we can produce nice plots showing how well the source list
#  matches the reference list.

from pkg_resources import require
require('numpy')
require('matplotlib')
from pylab import *
from numpy import *

#############################################################################

## @ingroup plotting
#    Utility function to read the positions of matching source and
#    reference points.  It returns, in order, the type of fit
#    (1=original match, 2=subsequent match), the ID, X and Y position,
#    the flux, the major and minor axes and the position angle for the
#    source point, and the matching reference point. This is designed to 
#    read from the "matches.txt" file produced by imageQualTest.
#    @param filename The file to read the data from
#    @return A list of numpy arrays as detailed above
def read_match_data(filename=None):
    """
    Utility function to read the positions of matching source and
    reference points.  It returns, in order, the type of fit
    (1=original match, 2=subsequent match), the ID, X and Y position,
    the flux, the major and minor axes and the position angle for the
    source point, and the matching reference point.  
    Usage:
    type,idS,xS,yS,fS,aS,bS,pS,idR,xR,yR,fR,aR,bR,pR = read_match_data("matches.txt")
    """
    type=[]
    idS=[]
    xS=[]
    yS=[]
    fS=[]
    aS=[]
    bS=[]
    pS=[]
    idR=[]
    xR=[]
    yR=[]
    fR=[]
    aR=[]
    bR=[]
    pR=[]
    for line in open(filename):
        fields = line.split()
        type.append(fields[0])
        idS.append(fields[1])
        xS.append(fields[2])
        yS.append(fields[3])
        fS.append(fields[4])
        aS.append(fields[5])
        bS.append(fields[6])
        pS.append(fields[7])
        idR.append(fields[8])
        xR.append(fields[9])
        yR.append(fields[10])
        fR.append(fields[11])
        aR.append(fields[12])
        bR.append(fields[13])
        pR.append(fields[14])
    
#    return cast[Int](array(type)),idS,cast[Float](array(xS)),cast[Float](array(yS)),cast[Float](array(fS)),cast[Float](array(aS)),cast[Float](array(bS)),cast[Float](array(pS)),idR,cast[Float](array(xR)),cast[Float](array(yR)),cast[Float](array(fR)),cast[Float](array(aR)),cast[Float](array(bR)),cast[Float](array(pR))
    return cast[int](array(type)),idS,cast[float](array(xS)),cast[float](array(yS)),cast[float](array(fS)),cast[float](array(aS)),cast[float](array(bS)),cast[float](array(pS)),idR,cast[float](array(xR)),cast[float](array(yR)),cast[float](array(fR)),cast[float](array(aR)),cast[float](array(bR)),cast[float](array(pR))
#    return array(type).astype(int),idS,array(xS).astype(float),array(yS).astype(float),array(fS).astype(float),array(aS).astype(float),array(bS).astype(float),array(pS).astype(float),idR,array(xR).astype(float),array(yR).astype(float),array(fR).astype(float),array(aR).astype(float),array(bR).astype(float),array(pR).astype(float)

#############################################################################

## @ingroup plotting
#   Utility function to read the positions of source and reference
#    points that weren't matched It returns, in order, the ID, X and Y
#    position of the source point, and ID, X & Y position of the
#    matching reference point. This is designed to read from the
#    "misses.txt" file produced by imageQualTest
#    @param filename The file to read from 
#    @return A list of numpy arrays as described above.

def read_miss_data(filename=None):
    """
    Utility function to read the positions of source and reference points that weren't matched
    It returns, in order, the ID, X and Y position of the source point, and ID, X & Y position of the matching reference point.
    Usage:
        type,id,x,y,f = read_miss_data("misses.txt")
    """
    type=[]
    id=[]
    x=[]
    y=[]
    f=[]
    for line in open(filename):
        fields = line.split()
        type.append(fields[0])
        id.append(fields[1])
        x.append(fields[2])
        y.append(fields[3])
        f.append(fields[4])
    
#    return type,id,cast[Float](array(x)),cast[Float](array(y)),cast[Float](array(f))
    return type,id,cast[float](array(x)),cast[float](array(y)),cast[float](array(f))
#    return type,id,array(x).astype(float),array(y).astype(float),array(f).astype(float)


