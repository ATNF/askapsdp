#!/usr/bin/env python
"""
"""
from pkg_resources import require
require('numpy')
require('matplotlib')
from pylab import *
from numpy import *

import os
root = os.environ["ASKAP_ROOT"]
sys.path.append(os.path.abspath(os.path.join(root,'Code/Components/Analysis/evaluation/trunk/plotting')))
from readData import *
from distributionPlots import *

if __name__ == '__main__':
    from sys import argv
    if len(argv) < 2:
        matchfile = 'matches.txt'
        missfile  = 'misses.txt'
    else:
        matchfile = argv[1]
        missfile = argv[2]

    matchType,idS,xS,yS,fS,aS,bS,pS,chisq,rms,ndof,npf,npo,idR,xR,yR,fR,aR,bR,pR = read_match_data(matchfile)
    missType,id,x,y,f = read_miss_data(missfile)

    print "Match list size = %d, Miss list size = %d (%d source and %d reference)"%(size(xS),size(x),size(missType[missType=='S']),size(missType[missType=='R']))

    figure(1, figsize=(16.5,11.7), dpi=72)

    posOffsetPlot(xS,yS,xR,yR,matchType)
    axisrange=spatPosPlot(xS,yS,xR,yR,matchType,x,y,missType,minRelVal=2.)
    
    spatHistPlot(fS,fR,xS,yS, axisrange, minRelVal=0., scaleByRel=True, absoluteSizes=False, removeZeros=True, name='S', unit='\mu Jy', locationCode=233, plotTitle='Flux difference')
    spatHistPlot(aS,aR,xS,yS, axisrange, minRelVal=0., scaleByRel=True, absoluteSizes=True, removeZeros=True, name='A', unit='\prime\prime', locationCode=234, plotTitle='Major axis difference')
    spatHistPlot(bS,bR,xS,yS, axisrange, minRelVal=0., scaleByRel=True, absoluteSizes=True, removeZeros=True, name='B', unit='\prime\prime', locationCode=235, plotTitle='Minor axis difference')
    PAspatHistPlot(aS,pS,pR,xS,yS, axisrange, minRelVal=1., removeZeros=True, locationCode=236)

    savefig('imageQualTest')

    show()

