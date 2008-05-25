#!/usr/bin/env python
"""
"""
from pylab import *
from numpy import *

def read_match_data(filename=None):
    """
    Utility function to read the positions of matching source and reference points.
    It returns, in order, the ID, X and Y position of the source point, and ID, X & Y position of the matching reference point.
    Usage:
        idS,xS,yS,idR,xR,yR = read_match_data("matches.txt")
    """
    idS=[]
    xS=[]
    yS=[]
    idR=[]
    xR=[]
    yR=[]
    for line in open(filename):
        fields = line.split()
        idS.append(fields[0])
        xS.append(fields[1])
        yS.append(fields[2])
        idR.append(fields[3])
        xR.append(fields[4])
        yR.append(fields[5])
    
    return idS,cast[Float](array(xS)),cast[Float](array(yS)),idR,cast[Float](array(xR)),cast[Float](array(yR))

def read_miss_data(filename=None):
    """
    Utility function to read the positions of source and reference points that weren't matched
    It returns, in order, the ID, X and Y position of the source point, and ID, X & Y position of the matching reference point.
    Usage:
        idS,xS,yS,idR,xR,yR = read_match_data("matches.txt")
    """
    origin=[]
    id=[]
    x=[]
    y=[]
    for line in open(filename):
        fields = line.split()
        origin.append(fields[0])
        id.append(fields[1])
        x.append(fields[2])
        y.append(fields[3])
    
    return origin,id,cast[Float](array(x)),cast[Float](array(y))



if __name__ == '__main__':
    from sys import argv
    if len(argv) < 1:
        matchfile = 'matches.txt'
        missfile  = 'misses.txt'
    else:
        matchfile = argv[1]
        missfile = argv[2]

    idS,xS,yS,idR,xR,yR = read_match_data(matchfile)
    origin,id,x,y = read_miss_data(missfile)

    dx = xS - xR
    dy = yS - yR

    meandx = mean(dx)
    meandy = mean(dy)

    figure(1)
    plot(dx,dy,'ro')
    axis('equal')
    axvline(color='k')
    axhline(color='k')
    axvline(meandx,color='r')
    axhline(meandy,color='r')
    xlabel(r'$\Delta x$ [arcsec]')
    ylabel(r'$\Delta y$ [arcsec]')

    figure(2)
    plot(xS,yS,'ro')
    xlabel('x [arcsec]')
    ylabel('y [arcsec]')
    for i in range(len(x)):
        if(origin[i]=='S'):
            plot([x[i]],[y[i]],'b+')
        else:
            plot([x[i]],[y[i]],'gx')
    
    show()
