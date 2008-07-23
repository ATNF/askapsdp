#!/usr/bin/env python
"""
"""
from pkg_resources import require
require('numpy')
require('matplotlib')
from pylab import *
from numpy import *

#############################################################################

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
    
    return cast[Int](array(type)),idS,cast[Float](array(xS)),cast[Float](array(yS)),cast[Float](array(fS)),cast[Float](array(aS)),cast[Float](array(bS)),cast[Float](array(pS)),idR,cast[Float](array(xR)),cast[Float](array(yR)),cast[Float](array(fR)),cast[Float](array(aR)),cast[Float](array(bR)),cast[Float](array(pR))

#############################################################################

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
    
    return type,id,cast[Float](array(x)),cast[Float](array(y)),cast[Float](array(f))


#############################################################################

def doHistSpatPlot(source=None, reference=None, xloc=None, yloc=None, removeZeros=False, position='b', name="X", plotTitle=""):
    """
    Utility function to plot, for a particular parameter, histograms
    of the absolute and relative differences in the values of the
    parameter between the source list and the reference list. The
    spatial positions of the sources are then plotted with symbols
    scaled by the size of the relative difference.
    """

    font = {'fontsize' : '10'}

    # Remove any src values of zero -- these have probably not been fitted and should be ignored
    ind = not(removeZeros) or (source != 0)
    src = source[ind]
    ref = reference[ind]
    xS  = xloc[ind]
    yS  = yloc[ind]

    diff = src - ref
    reldiff = diff / ref

    #####
    ## histogram of the absolute flux differences
    #####
    if(position=='b'):
        ax1=axes([0.125,0.1,0.35,0.16])
    else:
        ax1=axes([0.125,0.55,0.35,0.16])
    maxval = max(abs(max(diff)),abs(min(diff))) * 1.1
    n, bins, patches = hist(diff, 20)
    axis([-maxval,maxval,0,max(n)])
    ax = axis()
    xlabel(r'$\Delta %s$'%(name),font)
    ylabel('Count',font)
    mu=mean(diff)
    rms=std(diff)
    axvline(mu, color='r')
    print '%s (absolute):'%(plotTitle)
    print 'Mean = %f, RMS = %f'%(mu,rms)
    x = arange(ax[0],ax[1],(ax[1]-ax[0])/100.)
    y = normpdf( x, mu, rms)
    y = y * max(n) / max(y)
    l = plot(x, y, 'r--')
    mu=median(diff)
    adfm=abs(diff-mu)
    rms=median(adfm)
    axvline(mu, color='g')
    print 'Median = %f, MADFM = %f'%(mu,rms)
    y = normpdf( x, mu, rms/0.6744888)
    y = y * max(n) / max(y)
    l = plot(x, y, 'g--')
    axis(ax)

    #####
    ## histogram of the relative flux differences
    #####
    if(position=='b'):
        ax2=axes([0.125,0.31,0.35,0.16])
    else:
        ax2=axes([0.125,0.76,0.35,0.16])
    n, bins, patches = hist(reldiff, 20)
    reldiffmax = max(abs(max(reldiff)),abs(min(reldiff))) * 1.1
    axis([-reldiffmax,reldiffmax,0,max(n)])
    ax = axis()
    xlabel(r'$\Delta %s/%s_R$'%(name,name),font)
    ylabel('Count',font)
    title('%s: source - ref'%(plotTitle),font)
    mu=mean(reldiff)
    rms=std(reldiff)
    print '%s (relative):'%(plotTitle)
    print 'Mean = %7.3f%%, RMS = %7.3f%%'%(mu*100,rms*100)
    axvline(mu, color='r')
    x = arange(ax[0],ax[1],(ax[1]-ax[0])/100.)
    y = normpdf( x, mu, rms)
    y = y * max(n) / max(y)
    l = plot(x, y, 'r--')
    mu=median(reldiff)
    adfm=abs(reldiff-mu)
    rms=median(adfm)
    print 'Median = %7.3f%%, MADFM = %7.3f%%\n'%(mu*100,rms*100)
    axvline(mu, color='g')
    y = normpdf( x, mu, rms/0.6744888)
    y = y * max(n) / max(y)
    l = plot(x, y, 'g--')
    axis(ax)

    #####
    ## differences for matching sources as a function of their position
    #####
    if(position=='b'):
        subplot(224)
    else:
        subplot(222)
    tmp = -abs(reldiff)
    indSort = argsort(tmp)
    for i in indSort:
        size = 5. + abs(reldiff[i]/0.05) * 2.
        if(diff[i]>0):
            plot([xS[i]],[yS[i]],'ro',ms=size)
        else:
            plot([xS[i]],[yS[i]],'rs',ms=size)
    title(r'%s across field'%(plotTitle),font)
    xlabel(r'$x\ [\prime\prime]$',font)
    ylabel(r'$y\ [\prime\prime]$',font)
    axis(axisrange)

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
#    plot(xS,yS,'ro')
    tmp = -offset
    ind = argsort(tmp)
#    for i in range(len(xS)):
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


    doHistSpatPlot(fS, fR, xS, yS, True, name='F', plotTitle='Flux difference', position='b')

##    #####
##    ## Third figure: histogram of the absolute flux differences
##    #####
###    subplot(223)
##    ax1=axes([0.125,0.1,0.35,0.16])
##    dfmax = max(abs(max(df)),abs(min(df))) * 1.1
##    n, bins, patches = hist(df, 20)
##    axis([-dfmax,dfmax,0,max(n)])
##    ax = axis()
##    xlabel(r'$\Delta F$',font)
##    ylabel('Count',font)
###    title('Flux differences: source - ref')
##    mu=meandf
##    rms=std(df)
##    axvline(mu, color='r')
##    print 'Mean DF = ',mu,', RMS = ',rms
##    x = arange(ax[0],ax[1],(ax[1]-ax[0])/100.)
##    y = normpdf( x, mu, rms)
##    y = y * max(n) / max(y)
##    l = plot(x, y, 'r--')
##    mu=median(df)
##    adfm=abs(df-mu)
##    rms=median(adfm)
##    axvline(mu, color='g')
##    print 'Median DF = ',mu,', MADFM = ',rms
##    y = normpdf( x, mu, rms/0.6744888)
##    y = y * max(n) / max(y)
##    l = plot(x, y, 'g--')
##    axis(ax)
##    #####
##    ## Fourth figure: histogram of the relative flux differences
##    #####
##    axes([0.125,0.31,0.35,0.16])
##    n, bins, patches = hist(reldf, 20)
##    reldfmax = max(abs(max(reldf)),abs(min(reldf))) * 1.1
##    axis([-reldfmax,reldfmax,0,max(n)])
##    ax = axis()
##    xlabel(r'$\Delta F/F_R$',font)
##    ylabel('Count',font)
##    title('Flux differences: source - ref',font)
##    mu=meanreldf
##    rms=std(reldf)
##    axvline(mu, color='r')
##    print 'Mean DF/F = ',mu,', RMS = ',rms
##    x = arange(ax[0],ax[1],(ax[1]-ax[0])/100.)
##    y = normpdf( x, mu, rms)
##    y = y * max(n) / max(y)
##    l = plot(x, y, 'r--')
##    mu=median(reldf)
##    adfm=abs(reldf-mu)
##    rms=median(adfm)
##    axvline(mu, color='g')
##    print 'Median DF/F = ',mu,', MADFM = ',rms
##    y = normpdf( x, mu, rms/0.6744888)
##    y = y * max(n) / max(y)
##    l = plot(x, y, 'g--')
##    axis(ax)
##    #####
##    ## Fifth figure: flux differences for matching sources as a function of their position
##    #####
##    subplot(224)
##    xlabel(r'$x\ [\prime\prime]$',font)
##    ylabel(r'$y\ [\prime\prime]$',font)
##    title('Flux differences across field',font)
##    tmp = -abs(reldf)
##    ind = argsort(tmp)
###    for i in range(len(fS)):
##    for i in ind:
###        size = 5. + abs(df[i]/10.) * 3.
##        size = 5. + abs(reldf[i]/0.1) * 2.
##        if(df[i]>0):
##            plot([xS[i]],[yS[i]],'ro',ms=size)
##        else:
##            plot([xS[i]],[yS[i]],'rs',ms=size)
##    axis(axisrange)

    savefig('imageQualTest')

    figure(2, figsize=(10.,10.), dpi=72)
    font = {'fontsize' : '10'}
    rc('xtick', labelsize=8)
    rc('ytick', labelsize=8)
#    subplot(221)

    
##    ind = aS != 0
##    da = aS[ind] - aR[ind]
##    relda = da / aR[ind]
##
##    axes([0.125,0.55,0.35,0.16])
###    subplot(423)
##    n, bins, patches = hist(da, 20)
##    xlabel(r'$\Delta a$',font)
##    ylabel('Count',font)
##    mu=mean(da)
##    rms=std(da)
##    axvline(mu, color='r')
##    print 'Mean Da = ',mu,', RMS = ',rms
##    ax=axis()
##    x = arange(ax[0],ax[1],(ax[1]-ax[0])/100.)
##    y = normpdf( x, mu, rms)
##    y = y * max(n) / max(y)
##    l = plot(x, y, 'r--')
##    mu=median(da)
##    adfm=abs(da-mu)
##    rms=median(adfm)
##    axvline(mu, color='g')
##    print 'Median Da = ',mu,', MADFM = ',rms
##    y = normpdf( x, mu, rms/0.6744888)
##    y = y * max(n) / max(y)
##    l = plot(x, y, 'g--')
##    #####
##    ## Fourth figure: histogram of the relative major axis differences
##    #####
##    axes([0.125,0.76,0.35,0.16])
###    subplot(421)
##    n, bins, patches = hist(relda, 20)
##    xlabel(r'$\Delta A/A_R$',font)
##    ylabel('Count',font)
##    title('Major Axis differences: source - ref',font)
##    mu=mean(relda)
##    rms=std(relda)
##    axvline(mu, color='r')
##    print 'Mean DA/A = ',mu,', RMS = ',rms
##    ax=axis()
##    x = arange(ax[0],ax[1],(ax[1]-ax[0])/100.)
##    y = normpdf( x, mu, rms)
##    y = y * max(n) / max(y)
##    l = plot(x, y, 'r--')
##    mu=median(relda)
##    adfm=abs(relda-mu)
##    rms=median(adfm)
##    axvline(mu, color='g')
##    print 'Median DA/A = ',mu,', MADFM = ',rms
##    y = normpdf( x, mu, rms/0.6744888)
##    y = y * max(n) / max(y)
##    l = plot(x, y, 'g--')
##
##    #####
##    ## Fifth figure: major axis differences for matching sources as a function of their position
##    #####
##    subplot(222)
##    xlabel(r'$x\ [\prime\prime]$',font)
##    ylabel(r'$y\ [\prime\prime]$',font)
##    title('Minor axis differences across field',font)
##    tmp = -abs(relda)
##    ind = argsort(tmp)
###    for i in range(len(fS)):
##    for i in ind:
###        size = 5. + abs(df[i]/10.) * 3.
##        size = 5. + abs(relda[i]/0.1) * 2.
##        if(df[i]>0):
##            plot([xS[i]],[yS[i]],'ro',ms=size)
##        else:
##            plot([xS[i]],[yS[i]],'rs',ms=size)
##    axis(axisrange)

    doHistSpatPlot(aS, aR, xS, yS, True, 't', 'A', 'Major axis differences')
    doHistSpatPlot(bS, bR, xS, yS, True, 'b', 'B', 'Minor axis differences')

##    ind = bS != 0
##    db = bS[ind] - bR[ind]
##    reldb = db / bR[ind]
##
##    axes([0.125,0.1,0.35,0.16])
###    subplot(427)
##    n, bins, patches = hist(db, 20)
##    xlabel(r'$\Delta B$',font)
##    ylabel('Count',font)
##    mu=mean(db)
##    rms=std(db)
##    axvline(mu, color='r')
##    print 'Mean Db = ',mu,', RMS = ',rms
##    ax=axis()
##    x = arange(ax[0],ax[1],(ax[1]-ax[0])/100.)
##    y = normpdf( x, mu, rms)
##    y = y * max(n) / max(y)
##    l = plot(x, y, 'r--')
##    mu=median(db)
##    adfm=abs(db-mu)
##    rms=median(adfm)
##    axvline(mu, color='g')
##    print 'Median DB = ',mu,', MADFM = ',rms
##    y = normpdf( x, mu, rms/0.6744888)
##    y = y * max(n) / max(y)
##    l = plot(x, y, 'g--')
##    #####
##    ## Fourth figure: histogram of the relative minor axis differences
##    #####
##    axes([0.125,0.31,0.35,0.16])
###    subplot(425);
##    n, bins, patches = hist(reldb, 20)
##    xlabel(r'$\Delta B/B_R$',font)
##    ylabel('Count',font)
##    title('Minor Axis differences: source - ref',font)
##    mu=mean(reldb)
##    rms=std(reldb)
##    axvline(mu, color='r')
##    print 'Mean DB/B = ',mu,', RMS = ',rms
##    ax=axis()
##    x = arange(ax[0],ax[1],(ax[1]-ax[0])/100.)
##    y = normpdf( x, mu, rms)
##    y = y * max(n) / max(y)
##    l = plot(x, y, 'r--')
##    mu=median(reldb)
##    adfm=abs(reldb-mu)
##    rms=median(adfm)
##    axvline(mu, color='g')
##    print 'Median DB/B = ',mu,', MADFM = ',rms
##    y = normpdf( x, mu, rms/0.6744888)
##    y = y * max(n) / max(y)
##    l = plot(x, y, 'g--')
##
##    #####
##    ## Fifth figure: minor axis differences for matching sources as a function of their position
##    #####
##    subplot(224)
##    xlabel(r'$x\ [\prime\prime]$',font)
##    ylabel(r'$y\ [\prime\prime]$',font)
##    title('Minor axis differences across field',font)
##    tmp = -abs(reldb)
##    tempX = xS[ind]
##    tempY = yS[ind]
##    ind = argsort(tmp)
###    for i in range(len(fS)):
##    for i in ind:
###        size = 5. + abs(df[i]/10.) * 3.
##        size = 5. + abs(reldb[i]/0.1) * 2.
##        if(df[i]>0):
##            plot([tempX[i]],[tempY[i]],'ro',ms=size)
##        else:
##            plot([tempX[i]],[tempY[i]],'rs',ms=size)
##    axis(axisrange)



    show()

