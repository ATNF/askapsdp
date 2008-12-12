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
from numpy import *
import gc

font = {'fontsize' : '10'}

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

#############################################################################

## @ingroup plotting
#    @param array The array of values
#    @param name The name of the quantity (for writing to screen)
#    @param unit The units of the quantity

def drawNormalDist(array=None, name='X', unit=''):
    """
    Take an array, calculate the mean, rms, median and median absolute
    deviation from the median, and overplot Normal distributions for
    the two cases (mean/rms & median/madfm). In the robust case, the
    madfm is scaled by the madfmToRMS() function to the equivalent
    Gaussian rms. The values of these quantities are also written to screen. 

    Arguments:
       array: The array of values
       name: The name of the quantity (for writing to screen)
       unit: The units of the quantity
    """

    mu=mean(array) 
    rms=std(array) 
    med=median(array) 
    mad=madfm(array)  # median absolute deviation from median

    ax=axis()
    axvline(mu, color='r') 
    x = arange(ax[0],ax[1],(ax[1]-ax[0])/100.) 
    y = normpdf( x, mu, rms) 
    if(size(y)>0):
        y = y * ax[3] / max(y) 
    l = plot(x, y, 'r--') 
    axvline(med, color='g') 
    y = normpdf( x, med, madfmToRMS(mad)) 
    if(size(y)>0):
        y = y * ax[3] / max(y) 
    l = plot(x, y, 'g--') 
    axis(ax)

    print '%s:'%(name) 
    print 'Mean = %5.3f %s, RMS = %5.3f %s'%(mu,unit,rms,unit) 
    print 'Median = %5.3f %s, MADFM = %5.3f %s (or RMS=%5.3f)'%(med,unit,mad,unit,madfmToRMS(mad)) 
    

#############################################################################

## @ingroup plotting
#    @param locationCode The locationCode used by the subplot() function
#    @param side Either 'l' or 'r' (for left or right)
#    @return Array defining an axes location ([xmin,ymin,width,height])
def setBox(locationCode=231, side='l'):
    """
    A function to define where to place the histogram plot. The
    function starts with a subplot and a side, and returns an array
    suitable for use with axes() (ie. [xmin,ymin,width,height])

    Arguments:
       locationCode: The locationCode used by the subplot() function
       side: Either 'l' or 'r' (for left or right)
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

## @ingroup plotting
#    @param array The array of relative difference values.
#    @param locationCode Where on the page to place the histogram -- used by setBox()
#    @param name The name of the quantity    
def plotHistRel(array=None, locationCode=232, name="X"):
    """
    Draw a histogram of difference values given in the array. The
    horizontal axis label is written as the relative differences of
    the quantity. The location of the histogram is given by the
    setBox() function.

    Arguments:
       array: The array of relative difference values.
       locationCode: Where on the page to place the histogram -- used by setBox()
       name: The name of the quantity
    """

    axes(setBox(locationCode,'l'))
    maxval = max(abs(max(array)),abs(min(array))) * 1.1
    n, bins, patches = hist(array, 20)
    axis([-maxval,maxval,0,max(n)])
    lab = r'$100\times\Delta %s/%s_R$'%(name,name)
    xlabel(lab,font)


#############################################################################

## @ingroup plotting
#    @param array The array of absolute difference values.
#    @param locationCode Where on the page to place the histogram -- used by setBox()
#    @param name The name of the quantity
#    @param unit The units of the quantity
def plotHistAbs(array=None, locationCode=232, name="X", unit=""):
    """
    Draw a histogram of difference values given in the array. The
    horizontal axis label is written as the absolute differences of
    the quantity. The location of the histogram is given by the
    setBox() function.

    Arguments:
       array: The array of absolute difference values.
       locationCode: Where on the page to place the histogram -- used by setBox()
       name: The name of the quantity
       unit: The units of the quantity
    """

    axes(setBox(locationCode,'r'))
    maxval = max(abs(max(array)),abs(min(array))) * 1.1
    n, bins, patches = hist(array, 20)
    axis([-maxval,maxval,0,max(n)])
    xlabel(r'$\Delta %s [\rm{%s}]$'%(name,unit),font)
    

#############################################################################

## @ingroup plotting
#    @param source: List of values for the source objects
#    @param reference: List of values for the reference objects
#    @param xloc, yloc: x- and y- positions for the objects (either reference or source positions)
#    @param spatialAxis: An argument for the axis(...) function. Designed to be an array as produced by ax=axis(), such as the output from spatPosPlot().
#    @param scaleByRel: Scale the symbols by the relative difference (ie. (source-reference)/reference). If False, scale by the absolute difference
#    @param minRelVal: The minimum value, in multiples of the rms above the mean of the distribution being considered, that is plotted on the spatial plot. Points with values less than this are not plotted.
#    @param removeZeros: If True, all Source values that are zero are removed before plotting. This removes any spurious differences that may appear due to non-fitted components.
#    @param locationCode: The code used by subplot() to draw the graph, indicating where on the page the plot should go.
#    @param name: String with the quantity's name to be used in plotting
#    @param unit: String with the units of the quantity. Can be formatted in LaTeX for display purposes.
#    @param plotTitle: Title for overall plot
#    @param doHistRel: Draw the histogram of relative differences
#    @param doHistAbs: Draw the histogram of absolute differences
def spatHistPlot(source=None, reference=None, xloc=None, yloc=None, spatialAxis='auto', scaleByRel=True, minRelVal=2., absoluteSizes=True, sizeStep=5., removeZeros=False, locationCode=232, name="X", unit="", plotTitle="", doHistRel=True, doHistAbs=True):
    """
    Plots the spatial distribution of matched sources, with the size
    and shape of each point governed by the size of the difference of
    a particular quantity between the Source and Reference
    lists. Positive differences (in the sense of source-reference) are
    shown as circles, and negative ones as squares. Also shown are
    distributions of the absolute and relative differences of the
    quantities.

    Arguments:
       source: List of values for the source objects
       reference: List of values for the reference objects
       xloc, yloc: x- and y- positions for the objects (either reference or source positions)
       spatialAxis: An argument for the axis(...) function. Designed to be an array as produced 
                    by ax=axis(), such as the output from spatPosPlot().
       scaleByRel: Scale the symbols by the relative difference (ie. (source-reference)/reference).
                   If False, scale by the absolute difference
       minRelVal: The minimum value, in multiples of the rms above the mean of the distribution being 
                  considered, that is plotted on the spatial plot. Points with values less than this
                  are not plotted.
       removeZeros: If True, all Source values that are zero are removed before plotting. This
                    removes any spurious differences that may appear due to non-fitted components.
       locationCode: The code used by subplot() to draw the graph, indicating where on the page
                     the plot should go.
       name: String with the quantity's name to be used in plotting
       unit: String with the units of the quantity. Can be formatted in LaTeX for display purposes.
       plotTitle: Title for overall plot
       doHistRel: Draw the histogram of relative differences
       doHistAbs: Draw the histogram of absolute differences
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
        if(absoluteSizes):
            size = 3 + floor(abs(reldiff)/sizeStep)*2
        else:
            size = 3 + floor(abs((reldiff-mean(reldiff))/std(reldiff)))*2
    else:
        ind = argsort(-abs(diff))
        if(absoluteSizes):
            size = 3 + floor(abs(diff)/sizeStep)*2
        else:
            size = 3 + floor(abs((diff-mean(diff))/std(diff)))*2

    for i in ind:
        if(scaleByRel):
            thisval = abs((reldiff[i]-mean(reldiff))/std(reldiff))
        else:
            thisval = abs((diff[i]-mean(diff))/std(diff))
        if( thisval > minRelVal): 
            if(diff[i]>0):
                plot([xS[i]],[yS[i]],'ro',ms=size[i])
            else:
                plot([xS[i]],[yS[i]],'rs',ms=size[i])

    title(r'%s across field, difference >%3.1f$\sigma$'%(plotTitle,minRelVal),font)
    xlabel(r'$x\ [\prime\prime]$',font)
    ylabel(r'$y\ [\prime\prime]$',font)
    axis(spatialAxis)

    rc('xtick', labelsize=6)
    rc('ytick', labelsize=6)
    if(doHistRel):
        plotHistRel(reldiff,locationCode,name)
        drawNormalDist(reldiff,name+' (rel)','%')

    if(doHistAbs):
        plotHistAbs(diff,locationCode,name,unit)
        if(unit=='\prime\prime'):
            newunit='arcsec'
        else:
            newunit=unit
        drawNormalDist(diff,name+' (abs)',newunit)
    
#############################################################################

## @ingroup plotting
#    @param axisSrc The array of major axis values. This is used to tell whether a fit was not made, for use with the removeZeros parameter
#    @param paSrc The array of position angle values for the source list
#    @param paRef: Array of position angle values for the reference objects
#    @param xloc, yloc: x- and y- positions for the objects (either reference or source positions)
#    @param spatialAxis: An argument for the axis(...) function. Designed to be an array as produced by ax=axis(), such as the output from spatPosPlot().
#    @param minRelVal: The minimum value, in multiples of the rms above the mean of the distribution being considered, that is plotted on the spatial plot. Points with values less than this are not plotted.
#    @param removeZeros: If True, all Source values that are zero are removed before plotting. This removes any spurious differences that may appear due to non-fitted components.
#    @param locationCode: The code used by subplot() to draw the graph, indicating where on the page the plot should go.
def PAspatHistPlot(axisSrc=None, paSrc=None, paRef=None, xloc=None, yloc=None, spatialAxis='auto', minRelVal=2., absoluteSizes=True, removeZeros=False, locationCode=236,):
    """
    An optimised version of spatHistPlot for use with position angle
    values. These are special as the values can be zero even if the
    fit worked, and because, for the size of the symbols, a difference
    of 5 degrees and 175 degrees should be treated the same.  Net
    effect is a spatial distribution of matched sources, scaled in
    size according to the position angle difference (maximum size is
    at diff=90). Also drawn is a distribution of the absolute
    differences in position angle

    Arguments:
       axisSrc: List of major axis values that go with the position angle values for the source objects. Used to tell whether a fit was made or not (in combination with the removeZeros parameter).
       paSrc: List of position angle values for the source objects
       paRef: List of position angle values for the reference objects
       xloc, yloc: x- and y- positions for the objects (either reference or source positions)
       spatialAxis: An argument for the axis(...) function. Designed to be an array as produced 
                    by ax=axis(), such as the output from spatPosPlot().
       minRelVal: The minimum value, in multiples of the rms above the mean of the distribution being 
                  considered, that is plotted on the spatial plot. Points with values less than this
                  are not plotted.
       removeZeros: If True, all Source values that are zero are removed before plotting. This
                    removes any spurious differences that may appear due to non-fitted components.
       locationCode: The code used by subplot() to draw the graph, indicating where on the page
                     the plot should go.
       name: String with the quantity's name to be used in plotting
       unit: String with the units of the quantity. Can be formatted in LaTeX for display purposes.
       plotTitle: Title for overall plot
       doHistRel: Draw the histogram of relative differences
       doHistAbs: Draw the histogram of absolute differences
    """

    if(removeZeros):
        ind = axisSrc != 0
        src = paSrc[ind]
        ref = paRef[ind]
        x = xloc[ind]
        y = yloc[ind]
    else:
        src = paSrc
        ref = paRef
        x = xloc
        y = yloc

    paDiff = (src-ref+90)%180 - 90
    paNewRef = ref-ref

    spatHistPlot(paDiff,paNewRef,x,y,spatialAxis, removeZeros=False, scaleByRel=False, minRelVal=minRelVal, absoluteSizes=absoluteSizes, sizeStep=20., name='PA', unit='deg', locationCode=locationCode, plotTitle='Position angle difference',doHistRel=False, doHistAbs=False)

    if(removeZeros):
        ind = axisSrc != 0
        src = paSrc[ind]
        ref = paRef[ind]
    else:
        src = paSrc
        ref = paRef

    diff = src - ref

    plotHistAbs(diff,locationCode,'PA','deg')
    drawNormalDist(diff,'PA (abs)','deg')
    

#############################################################################

## @ingroup plotting
#    @param xS, yS: x- and y-positions of Source objects
#    @param xR, yR: x- and y-positions of Reference objects
#    @param flag: A numerical label indicating the quality of the match. Those with quality 1 will be coloured red, and those with other values magenta.
#    @param minPlottedOff: The minimum offset shown on the axes.
def posOffsetPlot(xS=None, yS=None, xR=None, yR=None, flag=None, minPlottedOff=5):
    """
    Plots the offsets in position of matched sources, along with
    circles of fixed radius and lines indicating the mean offset. 
    Arguments:
       xS, yS: x- and y-positions of Source objects
       xR, yR: x- and y-positions of Reference objects
       flag: A numerical label indicating the quality of the match. 
             Those with quality 1 will be coloured red, and those with other values magenta.
       minPlottedOff: The minimum offset shown on the axes.
    """
    
    rc('xtick', labelsize=10)
    rc('ytick', labelsize=10)

    subplot(231)
        
    dx = xS - xR
    dy = yS - yR


    print 'Overall mean offsets (x,y)=(%6.4f,%6.4f)'%(mean(dx),mean(dy)) 
    print '         rms offsets (x,y)=(%6.4f,%6.4f)'%(std(dx),std(dy)) 
    print 'Overall   mean offsets (x,y)=(%6.4f,%6.4f)'%(mean(dx),mean(dy)) 
    print '           rms offsets (x,y)=(%6.4f,%6.4f)'%(std(dx),std(dy)) 
    print '        median offsets (x,y)=(%6.4f,%6.4f)'%(median(dx),median(dy)) 
    print '         madfm offsets (x,y)=(%6.4f,%6.4f) = (%6.4f,%6.4f) as rms'%(madfm(dx),madfm(dy),madfmToRMS(madfm(dx)),madfmToRMS(madfm(dy))) 

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
    axvline(median(dx),color='g')
    axhline(median(dy),color='g')
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

    #Plot the 1-sigma ellipse for the points.
    plot(  std(dx)*cos(an)+mean(dx),   std(dy)*sin(an)+mean(dy), '-', color='r')
    plot(2*std(dx)*cos(an)+mean(dx), 2*std(dy)*sin(an)+mean(dy), '-', color='r')
    plot(  madfmToRMS(madfm(dx))*cos(an)+median(dx),   madfmToRMS(madfm(dy))*sin(an)+median(dy), '-', color='g')
    plot(2*madfmToRMS(madfm(dx))*cos(an)+median(dx), 2*madfmToRMS(madfm(dy))*sin(an)+median(dy), '-', color='g')
    axis(axisrange)


#############################################################################

## @ingroup plotting
#    @return An array describing the axis dimensions, as produced by ax=axis().
#    @param xS x-positions of Source objects
#    @param yS y-positions of Source objects
#    @param xR x-positions of Reference objects
#    @param yR y-positions of Reference objects
#    @param matchFlag Flags indicating quality of the match
#    @param xMiss x-positions of un-matched objects
#    @param yMiss y-positions of un-matched objects
#    @param missFlag A letter indicating the object type: 'R'=reference object, 'S'=source object
#    @param minRelVal: The minimum value, in multiples of the rms above the mean of relative offset, that is plotted on the spatial plot. Points with values less than this are not plotted.
#    @param minPlottedOff The minimum offset to be plotted on the histogram
def spatPosPlot(xS=None, yS=None, xR=None, yR=None, matchFlag=None, xMiss=None, yMiss=None, missFlag=None, minRelVal=2., minPlottedOff=5):
    """
    Plots the spatial location of all sources, indicating those that
    have been matched to the reference list, and those in either list
    that weren't matched. The matches are colour-coded according to
    the matchFlag array (a value of 1 is red, otherwise magenta), and
    their size scales with the offset in position. The Source objects
    not matched are indicated by blue crosses, while the Reference
    objects not matched are green plus-signs. Also shown in a
    histogram showing the distribution of offsets for the matching
    sources.
    Returns an array describing the axis dimensions.
    Arguments:
       xS, yS: x- and y-positions of Source objects
       xR, yR: x- and y-positions of Reference objects
       matchFlag: Flags indicating quality of the match
       xMiss, yMiss: x- and y-positions of un-matched objects
       missFlag: A letter indicating the object type: 'R'=reference object, 'S'=source object
       minRelVal: The minimum value, in multiples of the rms above the mean of relative offset,
                  that is plotted on the spatial plot. Points with values less than this are not plotted.
       minPlottedOff The minimum offset to be plotted on the histogram
    """

    rc('xtick', labelsize=10)
    rc('ytick', labelsize=10)

    bottomFrac = 0.25

    subplot(232)

    dx = xS - xR
    dy = yS - yR
    offset = sqrt(dx**2+dy**2)
    arglist = argsort(-offset)

    meandx = mean(dx)
    meandy = mean(dy)
    rmsdx = std(dx)
    rmsdy = std(dy)
    reloff = sqrt(((dx-meandx)/rmsdx)**2 + ((dy-meandy)/rmsdy)**2)
    arglist = argsort(-reloff)

    size = 2 + floor(reloff) * 2

    for i in arglist:
        if(reloff[i] > minRelVal):
            if(matchFlag[i]==1):
                plot([xS[i]],[yS[i]],'ro',ms=size[i])
            else:
                plot([xS[i]],[yS[i]],'mo',ms=size[i])
        else:
            if(matchFlag[i]==1):
                plot([xS[i]],[yS[i]],'r,')
            else:
                plot([xS[i]],[yS[i]],'m,')
           
    
    xlabel(r'$x\ [\prime\prime]$',font)
    ylabel(r'$y\ [\prime\prime]$',font)
#    title(r'Matches across field, offset >%3.1f$\sigma$'%(minRelVal),font)
    title(r'Matches and misses across field, offset >%3.1f$\sigma$'%(minRelVal),font)
    if(len(xMiss)>0):
        for i in range(len(xMiss)):
            #        if(missFlag[i]=='S'):
            #            plot([xMiss[i]],[yMiss[i]],'bx',ms=2)
            #        else:
            #            plot([xMiss[i]],[yMiss[i]],'g+')
            if(missFlag[i]=='S'):
                plot([xMiss[i]],[yMiss[i]],'b,')
            else:
                plot([xMiss[i]],[yMiss[i]],'g,')
        axis([min(min(xS),min(xMiss)),max(max(xS),max(xMiss)),min(min(yS),min(yMiss)),max(max(yS),max(yMiss))])
    else:
        axis([min(xS),max(xS),min(yS),max(yS)])
    ax = axis()
    axNew=[ax[0]-0.05*(ax[1]-ax[0]),ax[1]+0.05*(ax[1]-ax[0]),ax[2]-0.05*(ax[3]-ax[2]),ax[3]+0.05*(ax[3]-ax[2])]
    axis(axNew)
    ax = axis()
    aspectRatio = (ax[1]-ax[0])/(ax[3]-ax[2])
    axNew = [ax[0],ax[1],(ax[2]-bottomFrac*ax[3])/(1.-bottomFrac),ax[3]]
    step = ax[0]+aspectRatio*(ax[3]-axNew[2])-ax[1]
    axNew = [ax[0]-step/2., ax[1]+step/2., axNew[2], axNew[3]]
    axis(axNew)

    rc('xtick', labelsize=6)
    rc('ytick', labelsize=6)
    axes(setBox(232,'r'))
    n, bins, patches = hist(offset, max(minPlottedOff,max(offset))/0.25)
    ax=axis()
#    ax2=[ax[0],ax[1],ax[2],max(minPlottedOff,max(offset))]
#    axis(ax2)
    xlabel(r"$\rm{Offset} [\prime\prime]$",font)

    return axNew

    
