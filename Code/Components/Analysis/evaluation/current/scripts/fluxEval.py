#!/usr/bin/env python
"""
"""
import askap.analysis.evaluation
#from matplotlib import *
from pylab import *
from numpy import *
import os
from askap.analysis.evaluation.readData import *
from askap.analysis.evaluation.readDataOLD import *
from askap.analysis.evaluation.distributionPlots import *
from askap.analysis.evaluation.utils import *
from optparse import OptionParser
import askap.parset as parset

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
    minval = log10(min(xvals[xvals>0]))
    maxval = log10(max(xvals[xvals>0]))
    delta = (maxval-minval)/10.
    for i in range(10):
        xpt = (minval+delta/2.)+i*delta
        t = yvals[isGood[xvals>0] * (abs(log10(xvals[xvals>0]) - xpt)<delta/2.)]
        if len(t)>0:
            boxplot(t,positions=[10**xpt],widths=0.9*(10**(minval+(i+1)*delta)-10**(minval+i*delta)),sym='k.',patch_artist=False)
    semilogx(basex=10.)
#    axis([min(xvals)*0.9,max(xvals)*1.1,axisrange[2],axisrange[3]])
    xlim(min(xvals[xvals>0])*0.9,max(xvals[xvals>0])*1.1)

#############

if __name__ == '__main__':

    parser = OptionParser()
    parser.add_option("-c","--config", dest="inputfile", default="", help="Input parameter file [default: %default]")

    (options, args) = parser.parse_args()

    if(options.inputfile==''):
        inputPars = parset.ParameterSet()        
    elif(not os.path.exists(options.inputfile)):
        logging.warning("Config file %s does not exist!  Using default parameter values."%options.inputfile)
        inputPars = parset.ParameterSet()
    else:
        inputPars = parset.ParameterSet(options.inputfile).Eval

    sourceCatFile = inputPars.get_value('sourceCatalogue','')
    if(sourceCatFile == ''):
        logging.error('Eval.sourceCatalogue not provided. Doing no evaluation.')
        exit(0)
    if(not os.access(sourceCatFile,os.F_OK)):
        logging.error("Eval.sourceCatalogue %s does not exist. Doing no evaluation."%sourceCatFile)
        exit(0)
    sourceCatType = inputPars.get_value('sourceCatalogueType','Selavy')
    sourceCat = readCat(sourceCatFile,sourceCatType)
    
    refCatFile = inputPars.get_value('refCatalogue','')
    if(refCatFile == ''):
        logging.error('Eval.refCatalogue not provided. Doing no evaluation.')
        exit(0)
    if(not os.access(refCatFile,os.F_OK)):
        logging.error("Eval.refCatalogue %s does not exist. Doing no evaluation."%refCatFile)
        exit(0)
    refCatType = inputPars.get_value('refCatalogueType','Selavy')
    refCat = readCat(refCatFile,refCatType)

    matchfile = inputPars.get_value('matchfile',"matches.txt")
    if(not os.access(matchfile,os.F_OK)):
        logging.error("Match file %s does not exist. Doing no evaluation."%matchfile)
        exit(0)
    matchlist = readMatches(matchfile,sourceCat,refCat)
    
    missfile = inputPars.get_value('missfile',"misses.txt")
    if(not os.access(missfile,os.F_OK)):
        logging.error("Miss file %s does not exist. Doing no evaluation"%missfile)
        exit(0)
    srcmisslist = readMisses(missfile,sourceCat,'S')
    refmisslist = readMisses(missfile,refCat,'R')

    # Put data into individual arrays to match old plotting functions.
    xS=array([])
    yS=array([])
    imagerms=array([])
    fS=array([])
    fR=array([])
    aS=array([])
    bS=array([])
    pS=array([])
    npf=array([])
    npo=array([])
    ndof=array([])
    nfree=array([])
    rms=array([])
    chisq=array([])
    offset=array([])
    for m in matchlist:
        xS=append(xS,m.src.ra)
        yS=append(yS,m.src.dec)
        if sourceCatType=="Selavy" :
            imagerms=append(imagerms,m.src.RMSimage)
        fS=append(fS,m.src.flux())
        fR=append(fR,m.ref.flux())
        aS=append(aS,m.src.maj)
        bS=append(bS,m.src.min)
        pS=append(pS,m.src.pa)
        if sourceCatType=="Selavy" :
            npf=append(npf,m.src.npixFIT)
            npo=append(npf,m.src.npixObj)
            ndof=append(ndof,m.src.ndofFIT)
            nfree=append(nfree,m.src.nfreeFIT)
            rms=append(rms,m.src.rmsFIT)
            chisq=append(chisq,m.src.chisqFIT)
        offset=append(offset,m.sep)

    x=array([])
    y=array([])
    missType=array([])
    npo2=array([])
    imagerms2=array([])
    for m in srcmisslist:
        x=append(x,m.ra)
        y=append(y,m.dec)
        missType=append(missType,'S')
        if sourceCatType=="Selavy" :
            npo2=append(npo2,sourceCat[m.id].npixObj)
            imagerms2=append(imagerms2,sourceCat[m.id].RMSimage)
    for m in refmisslist:
        x=append(x,m.ra)
        y=append(y,m.dec)
        missType=append(missType,'R')
        if sourceCatType=="Selavy" :
            npo2=append(npo2,refCat[m.id].npixObj)
            imagerms2=append(imagerms2,refCat[m.id].RMSimage)
                         
    #    fluxScaling = 1.e6
    fluxScaling = 1.
    fS = fS * fluxScaling
    fR = fR * fluxScaling
    imagerms = imagerms * fluxScaling

    if(size(x)>0):
        print "Match list size = %d, Miss list size = %d (%d source and %d reference)"%(size(xS),size(x),size(missType[missType=='S']),size(missType[missType=='R']))
    else:
        print "Match list size = %d, Miss list size = %d"%(size(xS),size(x))

    dF = fS - fR
    rdF = zeros(dF.size)
    rdF[fR!=0] = 100.*dF[fR!=0]/fR[fR!=0]
    snr = ones(fS.size)
    if sourceCatType=="Selavy" :
        snr[imagerms>0] = fS[imagerms>0] / imagerms[imagerms>0]
    xSav=mean(xS)
    ySav=mean(yS)
    radius = sqrt((xS-xSav)*(xS-xSav)+(yS-ySav)*(yS-ySav))
    azimuth = arctan(abs(yS-ySav)/abs(xS-xSav)) * 180. / math.pi
    for i in range(len(azimuth)):
        if(yS[i]>ySav):
            if(xS[i]<xSav):
                azimuth[i] = 180. - azimuth[i]
        else:
            if(xS[i]<xSav):
                azimuth[i] = 180. + azimuth[i]
            else:
                azimuth[i] = 360. - azimuth[i]
    azimuth = azimuth % 360.
    area = math.pi * aS * bS / 4.
    numComp=ones(npf.size)
    if sourceCatType=="Selavy" :
        numComp[nfree>0] = (npf[nfree>0]-ndof[nfree>0]-1)/nfree[nfree>0]
    numNeighbours = zeros(len(xS))
    for i in range(len(xS)):
        for j in range(len(x)):
            if(missType[j]=='R'):
                dist = sqrt((x[j]-xS[i])*(x[j]-xS[i]) + (y[j]-yS[i])*(y[j]-yS[i]))
                if(dist<30./3600.):
                    numNeighbours[i]+=1

#################################################

    print "Fraction with |dS/S|<10%% = %5.2f%%"%(100.*size(rdF[abs(rdF)<10])/float(size(rdF)))
    print "Fraction with |dS/S|<20%% = %5.2f%%"%(100.*size(rdF[abs(rdF)<20])/float(size(rdF)))
    print "Fraction with |dS/S|<30%% = %5.2f%%"%(100.*size(rdF[abs(rdF)<30])/float(size(rdF)))
    print "Fraction with dS/S>30%%   = %5.2f%%"%(100.*size(rdF[rdF>30])/float(size(rdF)))
    print ""

    if sourceCatType=="Selavy" :
        dFgood = dF[npf>0]
    else:
        dFgood = dF
    print "Mean of dS   = %10.6f"%(mean(dFgood))
    print "Median of dS = %10.6f"%(median(dFgood))
    print "RMS of dS    = %10.6f"%(std(dFgood))
    print "MADFM of dS  = %10.6f  = %10.6f as RMS"%(madfm(dFgood),madfmToRMS(madfm(dFgood)))

    if sourceCatType=="Selavy" :
        print "Average of the ImageRMS values = %10.6f"%(mean(imagerms[npf>0]))
        print "Weighted average of the ImageRMS values = %10.6f"%(sum(imagerms[npf>0]*npf[npf>0])/(sum(npf[npf>0])*1.))

    if sourceCatType=="Selavy" :
        goodfit = npf>0
    else:
        goodfit = array(ones(fS.size),dtype=bool)
#    ind = argsort(rdF)[goodfit[argsort(rdF)]]
    ind = array(range(len(rdF)))[goodfit]

#################################################

    print "\nDoing plot of flux errors"

    figure(1, figsize=(16.5,11.7), dpi=72)

    font = {'fontsize' : '8'}
    legfont = {'fontsize' : '4'}
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

        #########################################

        plotcount = nextplot(plotcount)
        n, bins, patches = hist(arr[ind], 20)
        xlabel(lab,font)
        ylabel('Number',font)
        xticks(rotation=-30)

        plotcount = nextplot(plotcount)
        mu=median(arr[ind])
        sigma=madfmToRMS(madfm(arr[ind]))
        upper=mu+3.*sigma
        lower=mu-3.*sigma
        print "Median = %f, sigma=%f, so +-3sigma range is from %f to %f"%(mu,sigma,lower,upper)
        #        upper = int(ceil(upper/10)*10.)
        #lower = int(floor(lower/10)*10.)
        n, bins, patches = hist(arr[ind], 20, range=[lower,upper], normed=1)
        axisrange = axis()
        ytemp1 = normpdf(bins,mu,sigma)
        #        l1 = plot(bins, ytemp1, 'r-',label=r"$\Delta S$ mean&rms")
        l1 = plot(bins, ytemp1, 'r-')
        if loop==0 and sourceCatType=="Selavy" :
            ytemp2 = normpdf(bins,mu,mean(imagerms[npf>0]))
            #l2 = plot(bins, ytemp2*max(ytemp1)/max(ytemp2), 'g-', label="image rms")
            l2 = plot(bins, ytemp2*max(ytemp1)/max(ytemp2), 'g-')
        axisrange = axis()
        axis([lower,upper,axisrange[2],axisrange[3]])
        #        setp(l1, 'linewidth', 2)
        if loop==0 and sourceCatType=="Selavy" :
            setp(l2, 'linewidth', 2)
        xlabel(lab,font)
        xticks(rotation=-30)
        ylabel('Number',font)
        #legend()

#        plotcount = nextplot(plotcount)
#        temparr = arr[goodfit * (nfree==3)]
#        if((nfree==3).any()):
#            n, bins, patches = hist(temparr, bins=20, range=(min(arr[goodfit]),max(arr[goodfit])), fill=False, ec='red')
#        temparr = arr[goodfit * (nfree==6)]
#        if((nfree==6).any()):
#            n, bins, patches = hist(temparr, bins=20, range=(min(arr[goodfit]),max(arr[goodfit])), fill=False, ec='green')
#        xlabel(lab,font)
#        ylabel('Number',font)
#        xticks(rotation=-30)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([offset[i]],[arr[i]],'o')
        xlabel(r'Separation source-reference',font)
        ylabel(lab,font)

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

        #########################################
        
        plotcount = nextplot(plotcount)
        for i in ind:
            plot([fS[i]],[arr[i]],'o')
        xlabel(r'$S_{\rm Fit}$',font)
        ylabel(lab,font)
        xlim(min(fS[ind])*0.9,max(fS[ind])*1.1)
        yrange=ylim()
        xticks(rotation=-30)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([fS[i]],[arr[i]],'o')
        semilogx(basex=10.)
        xlabel(r'$\log_{10}(S_{\rm Fit})$',font)
        ylabel(lab,font)
        ylim(yrange)
        xlim(min(fS[ind])*0.9,max(fS[ind])*1.1)

        plotcount = nextplot(plotcount)
        bigBoxPlot(fS[ind],arr[ind],goodfit[ind])
        xlabel(r'$\log_{10}(S_{\rm Fit})$',font)
        ylabel(lab,font)        
        ylim(yrange)
        xlim(min(fS[ind])*0.9,max(fS[ind])*1.1)

        plotcount = nextplot(plotcount)
        for i in ind:
            plot([snr[i]],[arr[i]],'o')
        semilogx(basex=10.)
        #        axisrange = axis()
        #        axis([min(snr)*0.9,max(snr)*1.1,axisrange[2],axisrange[3]])
        xlabel(r'$\log_{10}(S/N (Fit))$',font)
        ylabel(lab,font)
        ylim(yrange)
        xlim(min(snr[ind])*0.9,max(snr[ind])*1.1)

        plotcount = nextplot(plotcount)
        bigBoxPlot(snr[ind],arr[ind],goodfit[ind])
        xlabel(r'$\log_{10}(S/N (Fit))$',font)
        ylabel(lab,font)        
        ylim(yrange)
        xlim(min(snr[ind])*0.9,max(snr[ind])*1.1)

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
        if sourceCatType=="Selavy" :
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
        if sourceCatType=="Selavy" :
            for i in ind:
                plot([rms[i]],[arr[i]],'o')
        xlabel(r'RMS of fit',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        if sourceCatType=="Selavy" :
            n, bins, patches = hist(chisq[ind], 20)
        xlabel(r'$\chi^2$ of fit',font)
        ylabel('Number',font)

        plotcount = nextplot(plotcount)
        if sourceCatType=="Selavy" :
            n, bins, patches = hist(chisq[ind]/ndof[ind], 20)
        xlabel(r'$\chi^2/\nu$ of fit',font)
        ylabel('Number',font)

        plotcount = nextplot(plotcount)
        if sourceCatType=="Selavy" :
            n, bins, patches = hist(area[ind], 20)
        xlabel(r'Area of fitted Gaussian',font)
        ylabel('Number',font)

        plotcount = nextplot(plotcount)
        if sourceCatType=="Selavy" :
            for i in ind:
                plot([npf[i]],[arr[i]],'o')
        xlabel(r'Number of pixels in fit',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        if sourceCatType=="Selavy" :
            for i in ind:
                plot([npo[i]],[arr[i]],'o')
        xlabel(r'Number of pixels in object',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        if sourceCatType=="Selavy" :
            for i in ind:
                plot([chisq[i]],[arr[i]],'o')
        xlabel(r'$\chi^2$ of fit',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        if sourceCatType=="Selavy" :
            for i in ind:
                plot([chisq[i]/ndof[i]],[arr[i]],'o')
        xlabel(r'$\chi^2/\nu$ of fit',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        if sourceCatType=="Selavy" :
            for i in ind:
                plot([area[i]],[arr[i]],'o')
        xlabel(r'Area of fitted Gaussian',font)
        ylabel(lab,font)

        plotcount = nextplot(plotcount)
        if sourceCatType=="Selavy" :
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

