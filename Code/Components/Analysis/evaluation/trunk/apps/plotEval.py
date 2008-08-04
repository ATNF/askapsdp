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

#############################################################################


if __name__ == '__main__':
    from sys import argv
    if len(argv) < 2:
        matchfile = 'matches.txt'
        missfile  = 'misses.txt'
    else:
        matchfile = argv[1]
        missfile = argv[2]

    matchType,idS,xS,yS,fS,aS,bS,pS,idR,xR,yR,fR,aR,bR,pR = read_match_data(matchfile)
    missType,id,x,y,f = read_miss_data(missfile)

    dx = xS - xR
    dy = yS - yR
    offset = sqrt(dx**2+dy**2)

    meandx = mean(dx)
    meandy = mean(dy)
    maxoffset = sqrt(max(dx)**2+max(dy)**2)

    print 'Overall mean offsets (x,y)=(%6.4f,%6.4f)\n'%(meandx,meandy)

    #####
    ## First figure: plotting the positional offsets for each match
    #####
    figure(1, figsize=(10.,10.), dpi=72)
    font = {'fontsize' : '10'}
    rc('xtick', labelsize=10)
    rc('ytick', labelsize=10)
    subplot(221)
    for i in range(len(dx)):
        if(matchType[i]==1):
            plot([dx[i]],[dy[i]],'ro')
        else:
            plot([dx[i]],[dy[i]],'mo')
    axis('equal')
    axisrange = axis()
    axvline(color='k')
    axhline(color='k')
    axvline(meandx,color='r')
    axhline(meandy,color='r')
    xlabel(r'$\Delta x\ [\prime\prime]$',font)
    ylabel(r'$\Delta y\ [\prime\prime]$',font)
    title('Positional offsets of matches',font)
    an = linspace(0,2*pi,100)
    plot( 1*cos(an), 1*sin(an), ':k' )
    plot( 2*cos(an), 2*sin(an), ':k' )
    plot( 4*cos(an), 4*sin(an), ':k' )
    plot( 6*cos(an), 6*sin(an), ':k' )
    plot( 8*cos(an), 8*sin(an), ':k' )
    plot(10*cos(an),10*sin(an), ':k' )
    axis(axisrange)


    #####
    ## Second figure: plotting the positions of matching and non-matching sources
    #####
    subplot(222)
    tmp = -offset
    ind = argsort(tmp)
    for i in ind:
        size = 5. + (floor(offset[i]/2.)) * 3.
        if(matchType[i]==1):
            plot([xS[i]],[yS[i]],'ro',ms=size)
        else:
            plot([xS[i]],[yS[i]],'mo',ms=size)
    xlabel(r'$x\ [\prime\prime]$',font)
    ylabel(r'$y\ [\prime\prime]$',font)
    title('Matches and misses across field',font)
    for i in range(len(x)):
        if(missType[i]=='S'):
            plot([x[i]],[y[i]],'bx')
        else:
            plot([x[i]],[y[i]],'g+')
    axisrange = axis()


    doHistSpatPlot(fS, fR, xS, yS, axisrange, True, name='F', plotTitle='Flux difference', position='b')

    savefig('imageQualTest1')

    figure(2, figsize=(10.,10.), dpi=72)
    font = {'fontsize' : '10'}
    rc('xtick', labelsize=8)
    rc('ytick', labelsize=8)

    doHistSpatPlot(aS, aR, xS, yS, axisrange, True, 't', 'A', 'Major axis differences')
    doHistSpatPlot(bS, bR, xS, yS, axisrange, True, 'b', 'B', 'Minor axis differences')
#    doHistSpatPlot(pS, pR, xS, yS, axisrange, True, 't', 'PA', 'Position angle differences')

    savefig('imageQualTest2')

    show()

