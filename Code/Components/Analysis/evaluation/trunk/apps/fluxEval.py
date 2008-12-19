#!/usr/bin/env python
"""
"""
from pkg_resources import require
#require('numpy==1.0.3.1')
require('numpy==1.1.1')
#require('matplotlib==0.90.1')
require('matplotlib==0.98.3')
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

    matchType,idS,xS,yS,fS,aS,bS,pS,chisq,rms,ndof,npf,npo,idR,xR,yR,fR,aR,bR,pR = read_match_data(matchfile)
    missType,id,x,y,f = read_miss_data(missfile)

    print "Match list size = %d, Miss list size = %d (%d source and %d reference)"%(size(xS),size(x),size(missType[missType=='S']),size(missType[missType=='R']))

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

    print "Mean of dS = %8.4f"%(mean(dF))
    print "Median of dS = %8.4f"%(median(dF))
    print "RMS of dS = %8.4f"%(std(dF))
    print "MADFM of dS = %8.4f = %8.4f as RMS"%(madfm(dF),madfmToRMS(madfm(dF)))

    figure(1, figsize=(16.5,11.7), dpi=72)

    font = {'fontsize' : '8'}
    rc('xtick', labelsize=8)
    rc('ytick', labelsize=8)

    subplots_adjust(wspace=0.3,hspace=0.3)

    goodfit = npf>0
    ind = []
    if(doRef):
        for i in argsort(rdF):
            if(hasRef[i] and goodfit[i]):
                ind.append(i)
    else:
        for i in argsort(rdF):
            if(goodfit[i]):
                ind.append(i)
    ind = array(ind)

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
    xlabel(r'$S_{\rm Fit}$',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs Fitted flux',font)

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

    numComp = (npf-ndof+1)/6
    subplot(455)
    for i in ind:
        plot([numComp[i]],[rdF[i]],'o')
    axisrange = axis()
    axis([0,5,axisrange[2],axisrange[3]])
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

    azimuth = arctan(yS/xS) * 180. / math.pi
    subplot(4,5,13)
    for i in ind:
        plot([azimuth[i]],[rdF[i]],'o')
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

    subplot(4,5,16)
    for i in ind:
        plot([aS[i]],[dF[i]],'o')
    xlabel(r'Major axis of fit [arcsec]',font)
    ylabel(r'$\Delta S$',font)
    title('Fitted flux vs major axis',font)

    subplot(4,5,17)
    temp=[]
    for i in ind:
        temp.append(dF[i])
    temp=array(temp)
    n, bins, patches = hist(temp, 20)
    xlabel(r'$\Delta S$',font)
    savefig('fluxEval')

    subplot(4,5,18)
    temp=[]
    for i in ind:
        temp.append(rdF[i])
    temp=array(temp)    
    n, bins, patches = hist(temp, 20)
    xlabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)

    subplot(4,5,19)
    logs = log10(fS)
    for i in ind:
        plot([logs[i]],[rdF[i]],'o')
    xlabel(r'$\log_{10}(S_{\rm fit})$',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs log S',font)


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
    axis([-0.5,2.5,axisrange[2],axisrange[3]])
    xlabel(r'No. of unmatched nearby catalogue sources',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs Num neighbours',font)

    if(doRef):
        savefig('fluxEvalRef')
    else:
        savefig('fluxEval')

    show()
    close()
    
