#!/usr/bin/env python

## @file
# A file containing functions to plot distributions (spatially and as histograms) of a given variable.

## @namespace distributionPlots
#  Plotting functions.
#  A set of functions to make quality evaluation plots, comparing a
#  source list to a reference list and looking at quantities such as
#  position, flux and spatial profile shape.

from pkg_resources import require
require('numpy')
require('matplotlib')
from pylab import *
import numpy
from numpy import *
from math import *
import gc
from utils import *
from modelcomponents import *

font = {'fontsize' : '10'}

#############################################################################

## @ingroup plotting
#    @param matchlist: List of Matched objects
#    @param flagSphericalPos: Are the positions in spherical coordinates? (If so, correct the RA offset by cos(dec)
#    @param spatialScale: Scale the offset positions by this factor (default=3600: deg-->arcsec)
#    @param minPlottedOff: The minimum offset shown on the axes.
def posOffsetPlotNew(matchlist=None, flagSphericalPos=True, spatialScale=3600., minPlottedOff=5.):
    """
    Plots the offsets in position of matched sources, along with
    circles of fixed radius and lines indicating the mean offset. 
    Arguments:
       matchlist: List of Matched objects
       flagSphericalPos: Are the positions in spherical coordinates? (If so, correct the RA offset by cos(dec)
       spatialScale: Scale the offset positions by this factor (default=3600: deg-->arcsec)
       minPlottedOff: The minimum offset shown on the axes.
    """
    
    rc('xtick', labelsize=10)
    rc('ytick', labelsize=10)

    subplot(331)

    dx = []
    dy = []
    for m in matchlist:
        m.setOffsets(flagSphericalPos)
        dx.append(m.dx*spatialScale)
        dy.append(m.dy*spatialScale)
        
    dx=array(dx)
    dy=array(dy)


    print 'Overall   mean offsets (x,y)=(%6.4f,%6.4f)'%(dx.mean(),dy.mean()) 
    print '           rms offsets (x,y)=(%6.4f,%6.4f)'%(dx.std(),dy.std()) 
    print '        median offsets (x,y)=(%6.4f,%6.4f)'%(median(dx),median(dy)) 
    print '         madfm offsets (x,y)=(%6.4f,%6.4f) = (%6.4f,%6.4f) as rms'%(madfm(dx),madfm(dy),madfmToRMS(madfm(dx)),madfmToRMS(madfm(dy))) 

    for m in matchlist:
        if(m.type == 1):
            plot(m.dx*spatialScale,m.dy*spatialScale,'ro')
        else:
            plot(m.dx*spatialScale,m.dy*spatialScale,'mo')
            
    axis('equal')
    axisrange = axis()
    axisrange = [min(-minPlottedOff,axisrange[0]), max(minPlottedOff,axisrange[1]),
                 min(-minPlottedOff,axisrange[2]), max(minPlottedOff,axisrange[3])]
    axvline(color='k')
    axhline(color='k')
    axvline(dx.mean(),color='r')
    axhline(dy.mean(),color='r')
    axvline(median(dx),color='g')
    axhline(median(dy),color='g')
    xlabel(r'$\Delta x\ [\prime\prime]$',font)
    ylabel(r'$\Delta y\ [\prime\prime]$',font)
    title('Positional offsets of matches',font)

    an = linspace(0,2*pi,100)
    #Plot the 1-sigma ellipse for the points.
    plot(  dx.std()*numpy.cos(an)+dx.mean(),   dy.std()*numpy.sin(an)+dy.mean(), '-', color='r')
    plot(2*dx.std()*numpy.cos(an)+dx.mean(), 2*dy.std()*numpy.sin(an)+dy.mean(), '-', color='r')
    plot(  madfmToRMS(madfm(dx))*numpy.cos(an)+median(dx),   madfmToRMS(madfm(dy))*numpy.sin(an)+median(dy), '-', color='g')
    plot(2*madfmToRMS(madfm(dx))*numpy.cos(an)+median(dx), 2*madfmToRMS(madfm(dy))*numpy.sin(an)+median(dy), '-', color='g')
    axis(axisrange)


#############################################################################

#    ## @ingroup plotting
#    #    @return An array describing the axis dimensions, as produced by ax=axis().
#    #    @param xS x-positions of Source objects
#    #    @param yS y-positions of Source objects
#    #    @param xR x-positions of Reference objects
#    #    @param yR y-positions of Reference objects
#    #    @param matchFlag Flags indicating quality of the match
#    #    @param xMiss x-positions of un-matched objects
#    #    @param yMiss y-positions of un-matched objects
#    #    @param missFlag A letter indicating the object type: 'R'=reference object, 'S'=source object
#    #    @param minRelVal: The minimum value, in multiples of the rms above the mean of relative offset, that is plotted on the spatial plot. Points with values less than this are not plotted.
#    #    @param minPlottedOff The minimum offset to be plotted on the histogram
#    def spatPosPlot(matchlist=None, misslist=None):
#        """
#        Plots the spatial location of all sources, indicating those that
#        have been matched to the reference list, and those in either list
#        that weren't matched. The matches are colour-coded according to
#        the matchFlag array (a value of 1 is red, otherwise magenta), and
#        their size scales with the offset in position. The Source objects
#        not matched are indicated by blue crosses, while the Reference
#        objects not matched are green plus-signs. Also shown in a
#        histogram showing the distribution of offsets for the matching
#        sources.
#        Returns an array describing the axis dimensions.
#        Arguments:
#           xS, yS: x- and y-positions of Source objects
#           xR, yR: x- and y-positions of Reference objects
#           matchFlag: Flags indicating quality of the match
#           xMiss, yMiss: x- and y-positions of un-matched objects
#           missFlag: A letter indicating the object type: 'R'=reference object, 'S'=source object
#           minRelVal: The minimum value, in multiples of the rms above the mean of relative offset,
#                      that is plotted on the spatial plot. Points with values less than this are not plotted.
#           minPlottedOff The minimum offset to be plotted on the histogram
#        """
#    
#        rc('xtick', labelsize=10)
#        rc('ytick', labelsize=10)
#    
#        bottomFrac = 0.25
#    
#        locationCode = 332
#        subplot(locationCode)
#    
#        dx = xS - xR
#        dy = yS - yR
#        offset = sqrt(dx**2+dy**2)
#        arglist = argsort(-offset)
#    
#        meandx = dx.mean()
#        meandy = dy.mean()
#        rmsdx = dx.std()
#        rmsdy = dy.std()
#        reloff = sqrt(((dx-meandx)/rmsdx)**2 + ((dy-meandy)/rmsdy)**2)
#        arglist = argsort(-reloff)
#    
#        size = 2 + floor(reloff) * 2
#    
#        for i in arglist:
#            if(reloff[i] > minRelVal):
#                if(matchFlag[i]==1):
#                    plot([xS[i]],[yS[i]],'ro',ms=size[i])
#                else:
#                    plot([xS[i]],[yS[i]],'mo',ms=size[i])
#            else:
#                if(matchFlag[i]==1):
#                    plot([xS[i]],[yS[i]],'r,')
#                else:
#                    plot([xS[i]],[yS[i]],'m,')
#               
#        
#        xlabel(r'$x\ [\prime\prime]$',font)
#        ylabel(r'$y\ [\prime\prime]$',font)
#    #    title(r'Matches across field, offset >%3.1f$\sigma$'%(minRelVal),font)
#        title(r'Matches and misses across field, offset >%3.1f$\sigma$'%(minRelVal),font)
#        if(len(xMiss)>0):
#            for i in range(len(xMiss)):
#                #        if(missFlag[i]=='S'):
#                #            plot([xMiss[i]],[yMiss[i]],'bx',ms=2)
#                #        else:
#                #            plot([xMiss[i]],[yMiss[i]],'g+')
#                if(missFlag[i]=='S'):
#                    plot([xMiss[i]],[yMiss[i]],'b,')
#                else:
#                    if(plotRefMisses):
#     		    plot([xMiss[i]],[yMiss[i]],'g,')
#            axis([min(min(xS),min(xMiss)),max(max(xS),max(xMiss)),min(min(yS),min(yMiss)),max(max(yS),max(yMiss))])
#        else:
#            axis([min(xS),max(xS),min(yS),max(yS)])
#        ax = axis()
#        axNew=[ax[0]-0.05*(ax[1]-ax[0]),ax[1]+0.05*(ax[1]-ax[0]),ax[2]-0.05*(ax[3]-ax[2]),ax[3]+0.05*(ax[3]-ax[2])]
#        axis(axNew)
#        ax = axis()
#        aspectRatio = (ax[1]-ax[0])/(ax[3]-ax[2])
#        axNew = [ax[0],ax[1],(ax[2]-bottomFrac*ax[3])/(1.-bottomFrac),ax[3]]
#        step = ax[0]+aspectRatio*(ax[3]-axNew[2])-ax[1]
#        axNew = [ax[0]-step/2., ax[1]+step/2., axNew[2], axNew[3]]
#        axis(axNew)
#    
#        rc('xtick', labelsize=6)
#        rc('ytick', labelsize=6)
#    #    axes(setBox(232,'r'))
#        axes(setBox(locationCode,'r'))
#        n, bins, patches = hist(offset, max(minPlottedOff,max(offset))/0.25)
#        ax=axis()
#    #    ax2=[ax[0],ax[1],ax[2],max(minPlottedOff,max(offset))]
#    #    axis(ax2)
#        xlabel(r"$\rm{Offset} [\prime\prime]$",font)
#    
#        return axNew

