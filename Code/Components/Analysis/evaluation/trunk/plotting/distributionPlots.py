#!/usr/bin/env python
"""
A file containing functions to plot distributions (spatially and as histograms) of a given variable.
"""
from pkg_resources import require
require('numpy')
require('matplotlib')
from pylab import *
from numpy import *

font = {'fontsize' : '10'}

#############################################################################

def setBox(locationCode=231, side='l'):
    """
    """

    xR = [0.15,0.42,0.69]
    xA = [0.26,0.53,0.80]
    y = [0.135,0.568]
    width = 0.08
    height = 0.05

    if(locationCode==232):
        if(side=='l'):
            return [xR[1],y[1],width,height]
        else:
            return [xA[1],y[1],width,height]
    elif(locationCode==233):
        if(side=='l'):
            return [xR[2],y[1],width,height]
        else:
            return [xA[2],y[1],width,height]
    elif(locationCode==234):
        if(side=='l'):
            return [xR[0],y[0],width,height]
        else:
            return [xA[0],y[0],width,height]
    elif(locationCode==235):
        if(side=='l'):
            return [xR[1],y[0],width,height]
        else:
            return [xA[1],y[0],width,height]
    elif(locationCode==236):
        if(side=='l'):
            return [xR[2],y[0],width,height]
        else:
            return [xA[2],y[0],width,height]

#############################################################################

def plotHistRel(array=None, locationCode=232, name="X"):

    axes(setBox(locationCode,'l'))
    maxval = max(abs(max(array)),abs(min(array))) * 1.1
    n, bins, patches = hist(array, 20)
    axis([-maxval,maxval,0,max(n)])
    xlabel(r'$\Delta %s/%s_R [%%]$'%(name,name),font)


#############################################################################

def plotHistAbs(array=None, locationCode=232, name="X", unit=""):

    axes(setBox(locationCode,'r'))
    maxval = max(abs(max(array)),abs(min(array))) * 1.1
    n, bins, patches = hist(array, 20)
    axis([-maxval,maxval,0,max(n)])
    xlabel(r'$\Delta %s [\rm{%s}]$'%(name,unit),font)
    


#############################################################################

def spatHistPlot(source=None, reference=None, xloc=None, yloc=None, spatialAxis='auto', scaleByRel=True, scaleStep=5., removeZeros=False, locationCode=232, name="X", unit="", plotTitle="", doHistRel=True, doHistAbs=True):
    """
    """
    
    font = {'fontsize' : '10'}
    rc('xtick', labelsize=10)
    rc('ytick', labelsize=10)

    # Remove any src values of zero -- these have probably not been fitted and should be ignored
    if(removeZeros):
        ind = source != 0
        src = source[ind]
        ref = reference[ind]
        xS  = xloc[ind]
        yS  = yloc[ind]
    else:
        src = source
        ref = reference
        xS = xloc
        yS = yloc

    diff = src - ref
    reldiff = 100. * diff / ref

    subplot(locationCode)

    if(scaleByRel):
        ind = argsort(-abs(reldiff))
        size = 5. + abs(reldiff/scaleStep) * 2.
    else:
        ind = argsort(-abs(diff))
        size = 5. + abs(diff/scaleStep) * 2.

    for i in ind:
        if(diff[i]>0):
            plot([xS[i]],[yS[i]],'ro',ms=size[i])
        else:
            plot([xS[i]],[yS[i]],'rs',ms=size[i])
    title(r'%s across field'%(plotTitle),font)
    xlabel(r'$x\ [\prime\prime]$',font)
    ylabel(r'$y\ [\prime\prime]$',font)
    axis(spatialAxis)

    rc('xtick', labelsize=6)
    rc('ytick', labelsize=6)
    if(doHistRel):
        plotHistRel(reldiff,locationCode,name)

    if(doHistAbs):
        plotHistAbs(diff,locationCode,name,unit)
    
#############################################################################

def posOffsetPlot(xS=None, yS=None, xR=None, yR=None, flag=None, minPlottedOff=5):
    """
    """
    
    rc('xtick', labelsize=10)
    rc('ytick', labelsize=10)

    subplot(231)
        
    dx = xS - xR
    dy = yS - yR

    for i in range(len(dx)):
        if(flag[i]==1):
            plot([dx[i]],[dy[i]],'ro')
        else:
            plot([dx[i]],[dy[i]],'mo')
    axis('equal')
    axisrange = axis()
    axisrange = [min(-minPlottedOff,axisrange[0]), max(minPlottedOff,axisrange[1]),
                 min(-minPlottedOff,axisrange[2]), max(minPlottedOff,axisrange[3])]
    axvline(color='k')
    axhline(color='k')
    axvline(mean(dx),color='r')
    axhline(mean(dy),color='r')
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


#############################################################################

def spatPosPlot(xS=None, yS=None, xR=None, yR=None, matchFlag=None, xMiss=None, yMiss=None, missFlag=None, minPlottedOff=5):
    """
    """
    font = {'fontsize' : '10'}
    rc('xtick', labelsize=10)
    rc('ytick', labelsize=10)

    bottomFrac = 0.25

    subplot(232)

    dx = xS - xR
    dy = yS - yR
    offset = sqrt(dx**2+dy**2)

    for i in argsort(-offset):
        size = 5. + (floor(offset[i]/2.)) * 3.
        if(matchFlag[i]==1):
            plot([xS[i]],[yS[i]],'ro',ms=size)
        else:
            plot([xS[i]],[yS[i]],'mo',ms=size)
    xlabel(r'$x\ [\prime\prime]$',font)
    ylabel(r'$y\ [\prime\prime]$',font)
    title('Matches and misses across field',font)
    for i in range(len(xMiss)):
        if(missFlag[i]=='S'):
            plot([xMiss[i]],[yMiss[i]],'bx')
        else:
            plot([xMiss[i]],[yMiss[i]],'g+')
    ax = axis()
    aspectRatio = (ax[1]-ax[0])/(ax[3]-ax[2])
    axNew = [ax[0],ax[1],(ax[2]-bottomFrac*ax[3])/(1.-bottomFrac),ax[3]]
    step = ax[0]+aspectRatio*(ax[3]-axNew[2])-ax[1]
    axNew = [ax[0]-step/2., ax[1]+step/2., axNew[2], axNew[3]]
    axis(axNew)

    rc('xtick', labelsize=6)
    rc('ytick', labelsize=6)
    axes(setBox(232,'l'))
    n, bins, patches = hist(offset, max(offset)/0.1)
    xlabel(r'$Offset [\prime\prime]$',font)

    return axNew

    
