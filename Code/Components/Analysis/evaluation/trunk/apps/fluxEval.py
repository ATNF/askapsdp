#!/usr/bin/env python
"""
"""
from pkg_resources import require
require('numpy')
require('matplotlib')
from pylab import *
from numpy import *
import numpy

import os
root = os.environ["ASKAP_ROOT"]
sys.path.append(os.path.abspath(os.path.join(root,'Code/Components/Analysis/evaluation/trunk/plotting')))
from readData import *
from distributionPlots import *

#from parameterset import *

#############
#global plotcount
def nextplot (plotcount):
    """ """
    subplot(4,5,plotcount)
    plotcount=plotcount+1
    return plotcount

#############

def bigBoxPlot (xvals, yvals, isGood, isLog=True):
    """ """
    minval = log10(min(xvals))
    maxval = log10(max(xvals))
    delta = (maxval-minval)/10.
    for i in range(10):
        xpt = (minval+delta/2.)+i*delta
        t = yvals[isGood * (abs(log10(xvals) - xpt)<delta/2.)]
        if len(t)>0:
            boxplot(t,positions=[10**xpt],widths=0.9*(10**(minval+(i+1)*delta)-10**(minval+i*delta)))
    semilogx(basex=10.)
    axis([min(xvals)*0.9,max(xvals)*1.1,axisrange[2],axisrange[3]])


#############

if __name__ == '__main__':
    from sys import argv

    matchfile = 'matches.txt'
    missfile  = 'misses.txt'

    matchType,idS,xS,yS,fS,aS,bS,pS,chisq,imagerms,rms,nfree,ndof,npf,npo,idR,xR,yR,fR,aR,bR,pR = read_match_data(matchfile)
    missType,id,x,y,f,chisq2,imagerms2,rms2,nfree2,ndof2,npf2,npo2 = read_miss_data(missfile)

    fluxScaling = 1.e6
    fS = fS * fluxScaling
    fR = fR * fluxScaling
    imagerms = imagerms * fluxScaling

    if(size(x)>0):
        print "Match list size = %d, Miss list size = %d (%d source and %d reference)"%(size(xS),size(x),size(missType[missType=='S']),size(missType[missType=='R']))
    else:
        print "Match list size = %d, Miss list size = %d"%(size(xS),size(x))

    dF = fS - fR
    rdF = 100.*dF/fR
    snr = fS / imagerms
    radius = sqrt(xS*xS+yS*yS)
    azimuth = arctan(abs(yS)/abs(xS)) * 180. / math.pi
    for i in range(len(azimuth)):
        if(yS[i]>0.):
            if(xS[i]<0.):
                azimuth[i] = 180. - azimuth[i]
        else:
            if(xS[i]<0.):
                azimuth[i] = 180. + azimuth[i]
            else:
                azimuth[i] = 360. - azimuth[i]
    azimuth = azimuth % 360.
    area = math.pi * aS * bS / 4.
    numComp = (npf-ndof+1)/nfree
    numNeighbours = zeros(len(xS))
    for i in range(len(xS)):
        for j in range(len(x)):
            if(missType[j]=='R'):
                dist = sqrt((x[j]-xS[i])*(x[j]-xS[i]) + (y[j]-yS[i])*(y[j]-yS[i]))
                if(dist<30.):
                    numNeighbours[i]+=1

#################################################

    print "Fraction with |dS/S|<10%% = %5.2f%%"%(100.*size(rdF[abs(rdF)<10])/float(size(rdF)))
    print "Fraction with |dS/S|<20%% = %5.2f%%"%(100.*size(rdF[abs(rdF)<20])/float(size(rdF)))
    print "Fraction with |dS/S|<30%% = %5.2f%%"%(100.*size(rdF[abs(rdF)<30])/float(size(rdF)))
    print "Fraction with dS/S>30%%   = %5.2f%%"%(100.*size(rdF[rdF>30])/float(size(rdF)))
    print ""

    dFgood = dF[npf>0]
    print "Mean of dS   = %10.6f"%(mean(dFgood))
    print "Median of dS = %10.6f"%(median(dFgood))
    print "RMS of dS    = %10.6f"%(std(dFgood))
    print "MADFM of dS  = %10.6f  = %10.6f as RMS"%(madfm(dFgood),madfmToRMS(madfm(dFgood)))
    print "Average of the ImageRMS values = %10.6f"%(mean(imagerms[npf>0]))
    print "Weighted average of the ImageRMS values = %10.6f"%(sum(imagerms[npf>0]*npf[npf>0])/(sum(npf[npf>0])*1.))

    goodfit = npf>0
#    ind = argsort(rdF)[goodfit[argsort(rdF)]]
    ind = array(range(len(rdF)))[goodfit]

#################################################

    print "\nDoing plot of flux errors"

    figure(1, figsize=(16.5,11.7), dpi=72)

    font = {'fontsize' : '8'}
    rc('xtick', labelsize=8)
    rc('ytick', labelsize=8)

    subplots_adjust(wspace=0.3,hspace=0.3)

    plotcount=1
    for loop in range(2):
        if(loop==0):
            arr = dF
            lab = r'$\Delta S$'
        else:
            arr = rdF
            percent='%'
            lab = r'$\Delta S/S_{\rm cat} [\%s]$'%percent

        plotcount = nextplot(plotcount)
        n, bins, patches = hist(arr[ind], 20)
        xlabel(lab,font)
        ylabel('Number',font)

        plotcount = nextplot(plotcount)
        mu=median(arr)
        sigma=madfmToRMS(madfm(arr))
        upper=mu+3.*sigma
        lower=mu-3.*sigma
        upper = int(ceil(upper/10)*10.)
        lower = int(floor(lower/10)*10.)
        n, bins, patches = hist(arr, 20, range=[lower,upper], normed=1)
        axisrange = axis()
        ytemp1 = normpdf(bins,mu,sigma)
        l1 = plot(bins, ytemp1, 'r-')
        if(loop==0):
            ytemp2 = normpdf(bins,mu,mean(imagerms[npf>0]))
            l2 = plot(bins, ytemp2*max(ytemp1)/max(ytemp2), 'g-')
        axisrange = axis()
        axis([lower,upper,axisrange[2],axisrange[3]])
        setp(l1, 'linewidth', 2)
        if(loop==0):
            setp(l2, 'linewidth', 2)
        xlabel(lab,font)
        ylabel('Number',font)

        plotcount = nextplot(plotcount)
        n, bins, patches = hist(arr[goodfit * (nfree==3)], 20, range=(arr.min(),arr.max()), fill=False, ec='red')
        n, bins, patches = hist(arr[goodfit * (nfree==6)], 20, range=(arr.min(),arr.max()), fill=False, ec='green')
        xlabel(lab,font)
        ylabel('Number',font)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([radius[i]],[arr[i]],'o')
        xlabel(r'Distance from field centre [arcsec]',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([azimuth[i]],[arr[i]],'o')
        axisrange=axis()
        axis([0.,360.,axisrange[2],axisrange[3]])
        xlabel(r'Azimuth around field centre [deg]',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([fS[i]],[arr[i]],'o')
        xlabel(r'$S_{\rm Fit}$',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([fS[i]],[arr[i]],'o')
        semilogx(basex=10.)
        axisrange = axis()
        axis([min(fS)*0.9,max(fS)*1.1,axisrange[2],axisrange[3]])
        xlabel(r'$\log_{10}(S_{\rm Fit})$',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        bigBoxPlot(fS,arr,goodfit)
        xlabel(r'$\log_{10}(S_{\rm Fit})$',font)
        ylabel(lab,font)        

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([snr[i]],[arr[i]],'o')
        semilogx(basex=10.)
        axisrange = axis()
        axis([min(snr)*0.9,max(snr)*1.1,axisrange[2],axisrange[3]])
        xlabel(r'$\log_{10}(S/N (Fit))$',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        bigBoxPlot(snr,arr,goodfit)
        xlabel(r'$\log_{10}(S/N (Fit))$',font)
        ylabel(lab,font)        

#    show()
    print "Saving to fluxEval.png"
    savefig('fluxEval')
    close()
    
#################################################

    for loop in range(2):
        if(loop==0):
            loopname="Absolute"
            arr = dF
            lab = r'$\Delta S$'
            figname = "fitEval_AbsErr"
        else:
            loopname="Relative"
            arr = rdF
            percent='%'
            lab = r'$\Delta S/S_{\rm cat} [\%s]$'%percent
            figname = "fitEval_RelErr"

        print "Doing plot of fit parameters for %s Flux Errors"%loopname

        figure(2, figsize=(16.5,11.7), dpi=72)

        font = {'fontsize' : '8'}
        rc('xtick', labelsize=8)
        rc('ytick', labelsize=8)

        subplots_adjust(wspace=0.3,hspace=0.3)

        plotcount=1

        plotcount = nextplot(plotcount)
        n, bins, patches = hist(aS[ind], 20)
        xlabel(r'Major axis of fit [arcsec]',font)
        ylabel('Number',font)

        plotcount = nextplot(plotcount)
        n, bins, patches = hist(bS[ind], 20)
        xlabel(r'Minor axis of fit [arcsec]',font)
        ylabel('Number',font)

        plotcount = nextplot(plotcount)
        n, bins, patches = hist(pS[ind], 20)
        xlabel(r'Position angle of fit [deg]',font)
        ylabel('Number',font)

        plotcount = nextplot(plotcount)
        n, bins, patches = hist(aS[ind]/bS[ind], 20)
        xlabel(r'Axial ratio of fit',font)
        ylabel('Number',font)

        plotcount = nextplot(plotcount)
        n, bins, patches = hist(rms[ind], 20)
        xlabel(r'RMS of fit',font)
        ylabel('Number',font)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([aS[i]],[arr[i]],'o')
        xlabel(r'Major axis of fit [arcsec]',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([bS[i]],[arr[i]],'o')
        xlabel(r'Minor axis of fit [arcsec]',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([pS[i]],[arr[i]],'o')
        xlabel(r'Position angle of fit [deg]',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([aS[i]/bS[i]],[arr[i]],'o')
        xlabel(r'Axial ratio of fit',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([rms[i]],[arr[i]],'o')
        xlabel(r'RMS of fit',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        n, bins, patches = hist(chisq[ind], 20)
        xlabel(r'$\chi^2$ of fit',font)
        ylabel('Number',font)

        plotcount = nextplot(plotcount)
        n, bins, patches = hist(chisq[ind]/ndof[ind], 20)
        xlabel(r'$\chi^2/\nu$ of fit',font)
        ylabel('Number',font)

        plotcount = nextplot(plotcount)
        n, bins, patches = hist(area[ind], 20)
        xlabel(r'Area of fitted Gaussian',font)
        ylabel('Number',font)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([npf[i]],[arr[i]],'o')
        xlabel(r'Number of pixels in fit',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([npo[i]],[arr[i]],'o')
        xlabel(r'Number of pixels in object',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([chisq[i]],[arr[i]],'o')
        xlabel(r'$\chi^2$ of fit',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([chisq[i]/ndof[i]],[arr[i]],'o')
        xlabel(r'$\chi^2/\nu$ of fit',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([area[i]],[arr[i]],'o')
        xlabel(r'Area of fitted Gaussian',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([numComp[i]],[arr[i]],'o')
        axisrange = axis()
        axis([0,max(numComp)+1,axisrange[2],axisrange[3]])
        xlabel(r'Number of fitted components',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([numNeighbours[i]],[arr[i]],'o')
        axisrange = axis()
        axis([-0.5,max(numNeighbours)+1.5,axisrange[2],axisrange[3]])
        xlabel(r'No. of unmatched nearby catalogue sources',font)
        ylabel(lab,font)

        print "Saving to %s.png"%figname
        savefig(figname)
        close()
    # end of: for loop in range(2)

#################################################

