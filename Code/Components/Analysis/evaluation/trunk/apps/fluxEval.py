#!/usr/bin/env python
"""
"""
from pkg_resources import require
require('numpy')
require('matplotlib')
#require('numpy==1.0.3.1')
#require('numpy==1.1.1')
#require('matplotlib==0.90.1')
#require('matplotlib==0.98.3')
from pylab import *
from numpy import *
import numpy

import os
root = os.environ["ASKAP_ROOT"]
sys.path.append(os.path.abspath(os.path.join(root,'Code/Components/Analysis/evaluation/trunk/plotting')))
from readData import *
from distributionPlots import *

#from parameterset import *

if __name__ == '__main__':
    from sys import argv

    matchfile = 'matches.txt'
    missfile  = 'misses.txt'

    doRef=False
    if(len(argv) == 2 and argv[1] == '-r'):
        doRef=True

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

    if(doRef):
        idRef = read_ref_list("reflist_200uJy_1deg.txt")
        hasRef = array(range(len(idS)))<0
        for s in range(len(idS)):
            for r in range(len(idRef)):
                if(idR[s]==idRef[r]):
                    print 'Matched Ref (%d/%d): %s\t%s\t%s'%(s,r,idS[s],idR[s],idRef[r])
                    hasRef[s] = True
    else:
        hasRef = array(range(len(idS)))>=0

    dF = fS - fR
    rdF = 100.*dF/fR

    print "Fraction with |dS/S|<10%% = %5.2f%%"%(100.*size(rdF[abs(rdF)<10])/cast[float](size(rdF)))
    print "Fraction with |dS/S|<20%% = %5.2f%%"%(100.*size(rdF[abs(rdF)<20])/cast[float](size(rdF)))
    print "Fraction with |dS/S|<30%% = %5.2f%%"%(100.*size(rdF[abs(rdF)<30])/cast[float](size(rdF)))
    print "Fraction with dS/S>30%%   = %5.2f%%"%(100.*size(rdF[rdF>30])/cast[float](size(rdF)))
    print ""

    dFgood = dF[npf>0]
    print "Mean of dS   = %10.6f"%(mean(dFgood))
    print "Median of dS = %10.6f"%(median(dFgood))
    print "RMS of dS    = %10.6f"%(std(dFgood))
    print "MADFM of dS  = %10.6f  = %10.6f as RMS"%(madfm(dFgood),madfmToRMS(madfm(dFgood)))
    print "Average of the ImageRMS values = %10.6f"%(mean(imagerms[npf>0]))
    print "Weighted average of the ImageRMS values = %10.6f"%(sum(imagerms[npf>0]*npf[npf>0])/(sum(npf[npf>0])*1.))

    figure(1, figsize=(16.5,11.7), dpi=72)

    font = {'fontsize' : '8'}
    rc('xtick', labelsize=8)
    rc('ytick', labelsize=8)

    subplots_adjust(wspace=0.3,hspace=0.3)

    goodfit = npf>0
    if(doRef):
        goodfit = goodfit * hasRef
    ind = argsort(rdF)[goodfit[argsort(rdF)]]

    if(doRef):
        print "Matches ordered by relative flux difference:"
        for i in ind:
            print '%d\t%s\t%s\t%6.3f\t%6.3f\t%d'%(i,idS[i],idR[i],rdF[i],aS[i],npf[i])

    subplot(451)
    for i in ind:
        plot([fS[i]],[dF[i]],'o')
    xlabel(r'$S_{\rm Fit}$',font)
    ylabel(r'$\Delta S$',font)
    title('Flux diff vs Fitted flux',font)

    subplot(452)
    for i in ind:
        plot([fS[i]],[rdF[i]],'o')
#        plot([fS[i]/imagerms[i]],[rdF[i]],'o')
    xlabel(r'$S_{\rm Fit}$',font)
#    xlabel(r'S/N (Fit)',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs Fitted flux',font)
#    title('Rel. Flux diff vs S/N',font)

    subplot(453)
    for i in ind:
        plot([chisq[i]],[rdF[i]],'o')
    xlabel(r'$\chi^2$ (fit)',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs chisq',font)

    subplot(454)
    for i in ind:
        plot([rms[i]],[rdF[i]],'o')
    xlabel(r'RMS of fit',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs fit RMS',font)

    numComp = (npf-ndof+1)/nfree
    subplot(455)
    for i in ind:
        plot([numComp[i]],[rdF[i]],'o')
    axisrange = axis()
    axis([0,max(numComp)+1,axisrange[2],axisrange[3]])
    xlabel(r'Number of fitted components',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs number of fitted components',font)

    subplot(456)
    for i in ind:
        plot([aS[i]],[rdF[i]],'o')
    xlabel(r'Major axis of fit [arcsec]',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs major axis',font)

    subplot(457)
    for i in ind:
            plot([bS[i]],[rdF[i]],'o')
    xlabel(r'Minor axis of fit [arcsec]',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs minor axis',font)

    subplot(458)
    for i in ind:
        plot([pS[i]],[rdF[i]],'o')
    xlabel(r'Position angle fit [arcsec]',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs PA',font)

    subplot(459)
    for i in ind:
            plot([aS[i]/bS[i]],[rdF[i]],'o')
    xlabel(r'Axial ratio of fit',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs axial ratio',font)

    fitarea = math.pi * aS * bS
    subplot(4,5,10)
    for i in ind:
        plot([fitarea[i]],[rdF[i]],'o')
    xlabel(r'Area of fitted Gaussian',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs area',font)

    subplot(4,5,11)
    for i in ind:
        plot([aS[i]],[fS[i]],'o')
    xlabel(r'Major axis of fit [arcsec]',font)
    ylabel(r'$S_{\rm fit}$',font)
    title('Fitted flux vs major axis',font)

    radius = sqrt(xS*xS+yS*yS)
    subplot(4,5,12)
    for i in ind:
        plot([radius[i]],[rdF[i]],'o')
    xlabel(r'Distance from field centre [arcsec]',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs distance from centre',font)

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
    subplot(4,5,13)
    for i in ind:
        plot([azimuth[i]],[rdF[i]],'o')
    axisrange=axis()
    axis([0.,360.,axisrange[2],axisrange[3]])
    xlabel(r'Azimuth around field centre [deg]',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs field azimuth',font)

    subplot(4,5,14)
    for i in ind:
        plot([npf[i]],[rdF[i]],'o')
    xlabel(r'Number of pixels in fit',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs num pixels in fit',font)

    subplot(4,5,15)
    for i in ind:
        plot([npo[i]],[rdF[i]],'o')
    xlabel(r'Number of pixels in object',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs num pixels in object',font)
#    for i in ind:
#        plot([npo[i]],[dF[i]],'o')
#    xlabel(r'Number of pixels in object',font)
#    ylabel(r'$\Delta S$',font)
#    title('Flux diff vs num pixels in object',font)

    subplot(4,5,16)
    for i in ind:
        plot([aS[i]],[dF[i]],'o')
    xlabel(r'Major axis of fit [arcsec]',font)
    ylabel(r'$\Delta S$',font)
    title('Flux diff vs major axis',font)

    subplot(4,5,17)
    temp=[]
    for i in ind:
        temp.append(dF[i])
    temp=array(temp)
    n, bins, patches = hist(temp, 20)
    xlabel(r'$\Delta S$',font)
    axes([0.35,0.18,0.05,0.05])
    mu=median(temp)
    sigma=madfmToRMS(madfm(temp))
    upper=mu+3.*sigma
    lower=mu-3.*sigma
    upper = cast[int](ceil(upper/10)*10.)
    lower = cast[int](floor(lower/10)*10.)
    n, bins, patches = hist(temp, 20, range=[lower,upper], normed=1)
    axisrange = axis()
    ytemp1 = normpdf(bins,mu,sigma)
    ytemp2 = normpdf(bins,mu,mean(imagerms[npf>0]))
    l1 = plot(bins, ytemp1, 'r-')
    l2 = plot(bins, ytemp2*max(ytemp1)/max(ytemp2), 'g-')
#    l2 = plot(bins, ytemp2, 'g-')
    axisrange = axis()
    axis([lower,upper,axisrange[2],axisrange[3]])
    xticks(cast[int]([lower,mu,upper]))
#    yticks([axisrange[3],axisrange[3]*3./4.,axisrange[3]/2.,axisrange[3]/4.])
    yticks([])
    setp(l1, 'linewidth', 2)
    setp(l2, 'linewidth', 2)


#    subplot(4,5,18)
#    temp=[]
#    for i in ind:
#        temp.append(rdF[i])
#    temp=array(temp)    
#    n, bins, patches = hist(temp, 20)
#    xlabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)

#    subplot(4,5,19)
    subplot(4,5,18)
    for i in ind:
#        plot([fS[i]/imagerms[i]],[rdF[i]],'o')
        plot([fS[i]/imagerms[i]],[dF[i]],'o')
    semilogx(basex=10.)
    axisrange = axis()
    axis([min(fS/imagerms)*0.9,max(fS/imagerms)*1.1,axisrange[2],axisrange[3]])
    xlabel(r'$\log_{10}(S/N (Fit))$',font)
#    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
#    title('Rel. Flux diff vs log(S/N)',font)
    ylabel(r'$\Delta S$',font)
    title('Flux diff vs log(S/N)',font)

    subplot(4,5,19)
    snrmin = log10(min(fS[ind]/imagerms[ind]))
    snrmax = log10(max(fS[ind]/imagerms[ind]))
    dSNR = (snrmax-snrmin)/10.
    print snrmin,snrmax,dSNR
    for i in range(10):
        snr = (snrmin+dSNR/2.)+i*dSNR
        t = dF[goodfit * (abs(log10(fS/imagerms) - snr)<dSNR/2.)]
        print 10**snr,median(t)
        if len(t)>0:
            boxplot(t,positions=[10**snr],widths=0.9*(10**(snrmin+(i+1)*dSNR)-10**(snrmin+i*dSNR)))
    semilogx(basex=10.)
    axis([min(fS/imagerms)*0.9,max(fS/imagerms)*1.1,axisrange[2],axisrange[3]])
    xlabel(r'$\log_{10}(S/N (Fit))$',font)
#    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
#    title('Rel. Flux diff vs log(S/N)',font)
    ylabel(r'$\Delta S$',font)
    title('Flux diff vs log(S/N)',font)

    numNeighbours = zeros(len(xS))
    for i in range(len(xS)):
        for j in range(len(x)):
            if(missType[j]=='R'):
                dist = sqrt((x[j]-xS[i])*(x[j]-xS[i]) + (y[j]-yS[i])*(y[j]-yS[i]))
                if(dist<30.):
                    numNeighbours[i]+=1
#        if((numNeighbours[i]==0) and (numComp[i]==1) and (aS[i]>0)):
#            print '%s\t%s\t%10.6f\t%7.3f'%(idS[i], idR[i], fS[i], rdF[i])
    subplot(4,5,20)
    for i in ind:
        plot([numNeighbours[i]],[rdF[i]],'o')
    axisrange = axis()
    axis([-0.5,max(numNeighbours)+1.5,axisrange[2],axisrange[3]])
    xlabel(r'No. of unmatched nearby catalogue sources',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs Num neighbours',font)

    if(doRef):
        savefig('fluxEvalRef')
    else:
        savefig('fluxEval')

#    show()
    close()
    
