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
    type,idS,xS,yS,fS,aS,bS,pS,chisq,imagerms,fitrms,ndof,npf,npo,idR,xR,yR,fR,aR,bR,pR = read_match_data("matches.txt")
    """
    type=[]
    idS=[]
    xS=[]
    yS=[]
    fS=[]
    aS=[]
    bS=[]
    pS=[]
    chisq=[]
    imagerms=[]
    rms=[]
    ndof=[]
    npixfit=[]
    npixobj=[]
    dudflux=[]
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
        chisq.append(fields[8])
        imagerms.append(fields[9])
        rms.append(fields[10])
        ndof.append(fields[11])
        npixfit.append(fields[12])
        npixobj.append(fields[13])
        dudflux.append(fields[14])
        idR.append(fields[15])
        xR.append(fields[16])
        yR.append(fields[17])
        fR.append(fields[18])
        aR.append(fields[19])
        bR.append(fields[20])
        pR.append(fields[21])
    
    return cast[int](array(type)),idS,cast[float](array(xS)),cast[float](array(yS)),cast[float](array(fS)),cast[float](array(aS)),cast[float](array(bS)),cast[float](array(pS)),cast[float](array(chisq)),cast[float](array(imagerms)),cast[float](array(rms)),cast[int](array(ndof)),cast[int](array(npixfit)),cast[int](array(npixobj)),idR,cast[float](array(xR)),cast[float](array(yR)),cast[float](array(fR)),cast[float](array(aR)),cast[float](array(bR)),cast[float](array(pR))

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
    
    return array(type),array(id),cast[float](array(x)),cast[float](array(y)),cast[float](array(f))


def read_ref_list(filename=None):

    id=[]
    for line in open(filename):
        fields = line.split()
        id.append('[%s_%s_%s]'%(fields[0],fields[1],fields[2]))

    return id
