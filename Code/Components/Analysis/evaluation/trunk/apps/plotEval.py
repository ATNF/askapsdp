#!/usr/bin/env python
"""
"""
from pkg_resources import require
require('numpy')
require('matplotlib')
from pylab import *
from numpy import *

def read_match_data(filename=None):
    """
    Utility function to read the positions of matching source and reference points.
    It returns, in order, the ID, X and Y position of the source point, and ID, X & Y position of the matching reference point.
    Usage:
        idS,xS,yS,fS,idR,xR,yR,fR = read_match_data("matches.txt")
    """
    idS=[]
    xS=[]
    yS=[]
    fS=[]
    idR=[]
    xR=[]
    yR=[]
    fR=[]
    for line in open(filename):
        fields = line.split()
        idS.append(fields[0])
        xS.append(fields[1])
        yS.append(fields[2])
        fS.append(fields[3])
        idR.append(fields[4])
        xR.append(fields[5])
        yR.append(fields[6])
        fR.append(fields[7])
    
    return idS,cast[Float](array(xS)),cast[Float](array(yS)),cast[Float](array(fS)),idR,cast[Float](array(xR)),cast[Float](array(yR)),cast[Float](array(fR))

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



if __name__ == '__main__':
    from sys import argv
    if len(argv) < 2:
        matchfile = 'matches.txt'
        missfile  = 'misses.txt'
    else:
        matchfile = argv[1]
        missfile = argv[2]

    idS,xS,yS,fS,idR,xR,yR,fR = read_match_data(matchfile)
    type,id,x,y,f = read_miss_data(missfile)

    dx = xS - xR
    dy = yS - yR
    offset = sqrt(dx**2+dy**2)
    df = fS - fR
    reldf = df/fR

    meandx = mean(dx)
    meandy = mean(dy)
    maxoffset = sqrt(max(dx)**2+max(dy)**2)
    meandf = mean(df)
    meanreldf = mean(reldf)

    figure(1, figsize=(10.,10.), dpi=72)
    font = {'fontsize' : '10'}
    rc('xtick', labelsize=8)
    rc('ytick', labelsize=8)
    subplot(221)
    plot(dx,dy,'ro')
    axis('equal')
    axvline(color='k')
    axhline(color='k')
    axvline(meandx,color='r')
    axhline(meandy,color='r')
    xlabel(r'$\Delta x\ [\prime\prime]$',font)
    ylabel(r'$\Delta y\ [\prime\prime]$',font)
    title('Positional offsets of matches',font)
    an = linspace(0,2*pi,100)
    plot( 2*cos(an), 2*sin(an), ':k' )
    plot( 4*cos(an), 4*sin(an), ':k' )
    plot( 6*cos(an), 6*sin(an), ':k' )


#    figure(2)
    subplot(222)
#    plot(xS,yS,'ro')
    tmp = -offset
    ind = argsort(tmp)
#    for i in range(len(xS)):
    for i in ind:
        size = 5. + (floor(offset[i]/2.)) * 3.
        plot([xS[i]],[yS[i]],'ro',ms=size)
    xlabel(r'$x\ [\prime\prime]$',font)
    ylabel(r'$y\ [\prime\prime]$',font)
    title('Matches and misses across field',font)
    for i in range(len(x)):
        if(type[i]=='S'):
            plot([x[i]],[y[i]],'gx')
        else:
            plot([x[i]],[y[i]],'b+')
    axisrange = axis()

#    subplot(223)
    axes([0.125,0.1,0.35,0.16])
    n, bins, patches = hist(df, 20)
    axvline(meandf, color='r')
    xlabel(r'$\Delta F$',font)
    ylabel('Count',font)
#    title('Flux differences: source - ref')
    axes([0.125,0.31,0.35,0.16])
    n, bins, patches = hist(reldf, 20)
    axvline(meanreldf, color='r')
    xlabel(r'$\Delta F/F$',font)
    ylabel('Count',font)
    title('Flux differences: source - ref',font)

    subplot(224)
    xlabel(r'$x\ [\prime\prime]$',font)
    ylabel(r'$y\ [\prime\prime]$',font)
    title('Flux differences across field',font)
    tmp = -reldf
    ind = argsort(tmp)
#    for i in range(len(fS)):
    for i in ind:
#        size = 5. + abs(df[i]/10.) * 3.
        size = 5. + abs(reldf[i]/0.1) * 3.
        if(df[i]>0):
            plot([xS[i]],[yS[i]],'ro',ms=size)
        else:
            plot([xS[i]],[yS[i]],'rs',ms=size)
    axis(axisrange)

    savefig('imageQualTest')

    show()
