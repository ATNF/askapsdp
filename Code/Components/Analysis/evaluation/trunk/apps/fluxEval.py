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

    matchType,idS,xS,yS,fS,aS,bS,pS,chisq,rms,ndof,idR,xR,yR,fR,aR,bR,pR = read_match_data(matchfile)
    missType,id,x,y,f = read_miss_data(missfile)

    print "Match list size = %d, Miss list size = %d"%(size(xS),size(x))

    figure(1, figsize=(16.5,11.7), dpi=72)

    dF = fS - fR
    rdF = 100.*dF/fR

    indFit = argsort(aS[aS>0])
    indNoFit = argsort(aS[aS==0.])

    font = {'fontsize' : '9'}
    rc('xtick', labelsize=9)
    rc('ytick', labelsize=9)

    subplot(341)
    for i in argsort(rdF):
        if(aS[i]>0):
            plot([fS[i]],[dF[i]],'o')
#        else:
#            plot([fS[i]],[dF[i]],'x')
    xlabel(r'$S_{\rm Fit}$',font)
    ylabel(r'$\Delta S$',font)
    title('Flux diff vs Fitted flux',font)

    subplot(342)
    for i in argsort(rdF):
        if(aS[i]>0):
            plot([fS[i]],[rdF[i]],'o')
#        else:
#            plot([fS[i]],[rdF[i]],'x')
    xlabel(r'$S_{\rm Fit}$',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs Fitted flux',font)

    subplot(343)
    for i in argsort(rdF):
        if(aS[i]>0):
            plot([chisq[i]],[rdF[i]],'o')
    xlabel(r'$\chi^2$ (fit)',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs chisq',font)

    subplot(344)
    for i in argsort(rdF):
        if(aS[i]>0):
            plot([rms[i]],[rdF[i]],'o')
    xlabel(r'RMS of fit',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs fit RMS',font)

    subplot(345)
    for i in argsort(rdF):
        if(aS[i]>0):
            plot([aS[i]],[rdF[i]],'o')
    xlabel(r'Major axis of fit [arcsec]',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs major axis',font)

    subplot(346)
    for i in argsort(rdF):
        if(aS[i]>0):
            plot([bS[i]],[rdF[i]],'o')
    xlabel(r'Minor axis of fit [arcsec]',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs minor axis',font)

    subplot(347)
    for i in argsort(rdF):
        if(aS[i]>0):
            plot([pS[i]],[rdF[i]],'o')
    xlabel(r'Position angle fit [arcsec]',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs PA',font)

    subplot(348)
    for i in argsort(rdF):
        if(aS[i]>0):
            plot([aS[i]/bS[i]],[rdF[i]],'o')
    xlabel(r'Axial ratio of fit',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs axial ratio',font)

    fitarea = math.pi * aS * bS
    subplot(349)
    for i in argsort(rdF):
        if(aS[i]>0):
            plot([fitarea[i]],[rdF[i]],'o')
    xlabel(r'Area of fitted Gaussian',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs area',font)

    subplot(3,4,10)
    for i in argsort(rdF):
        if(aS[i]>0):
            plot([aS[i]],[fS[i]],'o')
    xlabel(r'Major axis of fit [arcsec]',font)
    ylabel(r'$S_{\rm fit}$',font)
    title('Fitted flux vs major axis',font)

    radius = sqrt(xS*xS+yS*yS)
    subplot(3,4,11)
    for i in argsort(rdF):
        if(aS[i]>0):
            plot([radius[i]],[rdF[i]],'o')
#        else:
#            plot([radius[i]],[rdF[i]],'x')
    xlabel(r'Distance from field centre [arcsec]',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs distance from centre',font)

    azimuth = arctan(yS/xS) * 180. / math.pi
    subplot(3,4,12)
    for i in argsort(rdF):
        if(aS[i]>0):
            plot([azimuth[i]],[rdF[i]],'o')
#        else:
#           plot([azimuth[i]],[rdF[i]],'x')
    xlabel(r'Azimuth around field centre [deg]',font)
    ylabel(r'$\Delta S/S_{\rm Cat} [\%]$',font)
    title('Rel. Flux diff vs field azimuth',font)


    savefig('fluxEval')

    show()

