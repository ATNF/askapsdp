#!/usr/bin/env python

import askap.analysis.evaluation

import numpy as np
import matplotlib
matplotlib.use('Agg')
import pylab as plt
import math
import scipy.special
from askap.analysis.evaluation.readData import *
import askap.analysis.evaluation.modelcomponents as models
from askap.analysis.evaluation.sourceSelection import *
import pyfits
import pywcs
import os
from optparse import OptionParser
import askap.parset as parset

def labelPlot(xlab, ylab, title,textsize):
    plt.tick_params(labelsize=textsize)
    plt.xlabel(xlab,fontsize=textsize)
    plt.ylabel(ylab,fontsize=textsize)
    plt.title(title,fontsize=textsize)

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

    threshImageName=inputPars.get_value('thresholdImage','detectionThreshold.i.clean.fits')
    noiseImageName=inputPars.get_value('noiseImage','noiseMap.i.clean.fits')
    snrImageName=inputPars.get_value('snrImage','snr.i.clean.fits')
    skymodelCatalogue=inputPars.get_value('refCatalogue','skyModel-catalogue.txt')
    skymodelCatType = inputPars.get_value('refCatalogueType','Selavy')
    skymodelOrigCat=inputPars.get_value('origCatalogue','')
    skymodelOrigCatIsPrecessed=inputPars.get_value('origCatalogueIsPrecessed','false')
    sourceCatalogue=inputPars.get_value('sourceCatalogue','selavy-fitResults.txt')
    sourceCatType = inputPars.get_value('sourceCatalogueType','Selavy')
    matchfile = inputPars.get_value('matchfile','matches.txt')

    
    if not os.path.exists(threshImageName):
        print "Threshold image %s does not exist. Exiting."%threshImageName
        exit(0)
    if not os.path.exists(noiseImageName):
        print "Noise image %s does not exist. Exiting."%noiseImageName
        exit(0)
    if not os.path.exists(threshImageName):
        print "SNR image %s does not exist. Exiting."%snrImageName
        exit(0)
    
    threshim=pyfits.open(threshImageName)
    threshmapFull=threshim[0].data
    goodpixels=threshmapFull>0.
    threshmap = threshmapFull[goodpixels]
    threshHeader = threshim[0].header
    threshWCS = pywcs.WCS(threshHeader)
    threshim.close()

    # Tools used to determine whether a given missed reference source should be included
    selector = sourceSelector(inputPars)
    # this one is used for the original catalogue when it has not been precessed.
    selectorUnprecessed = sourceSelector(inputPars)
    selectorUnprecessed.setWCSreference(0.,0.)
    
    noiseim=pyfits.open(noiseImageName)
    noisemapFull=noiseim[0].data
    noisemap = noisemapFull[goodpixels]
    noiseim.close()

    snrim = pyfits.open(snrImageName)
    snrmapFull=snrim[0].data
    snrmap = snrmapFull[goodpixels]
    snrim.close()

#########################################
#  Noise map distribution

    plt.figure(num=2, figsize=(12.,12.), dpi=72)
    plt.subplots_adjust(wspace=0.4, hspace=0.6)

    plt.subplot(321)
    plt.hist(noisemap,bins=50)
    labelPlot('Pixel value','Count','Noise map histogram','x-small')
    
    plt.subplot(322)
    plt.hist(noisemap,bins=50,log=True)
    labelPlot('Pixel value','Count','Noise map histogram','x-small')
    

#########################################
#  Threshold map distribution

    plt.subplot(323)
    plt.hist(threshmap,bins=50)
    labelPlot('Pixel value','Count','Threshold map histogram','x-small')

    plt.subplot(324)
    plt.hist(threshmap,bins=50,log=True)
    labelPlot('Pixel value','Count','Threshold map histogram','x-small')

#########################################
#  SNR map distribution

    plt.subplot(325)
    plt.hist(snrmap[abs(snrmap)<100],bins=50)
    labelPlot('Pixel value','Count','SNR map histogram','x-small')

    plt.subplot(326)
    plt.hist(snrmap[abs(snrmap)<100],bins=50,log=True)
    labelPlot('Pixel value','Count','SNR map histogram','x-small')

    plt.savefig('histograms.png')

#########################################
#  SNR map distribution

    numPixAboveSNR=[]
    numPixBelowNegSNR=[]
    logSNRlevel=np.arange(5*math.log10(20.)+1)/5.
    snrLevel=10**logSNRlevel
    for snr in snrLevel:
        numPixAboveSNR.append(snrmap[snrmap>snr].size)
        numPixBelowNegSNR.append(snrmap[snrmap<-snr].size)

    numPixAboveSNR=np.array(numPixAboveSNR,dtype=float)
    numPixBelowNegSNR=np.array(numPixBelowNegSNR,dtype=float)

    plt.figure(num=3,figsize=(8,8),dpi=72)
    plt.loglog()
    plt.plot(snrLevel,numPixAboveSNR,'b-',lw=3,label='Positive signal')
    plt.plot(snrLevel,numPixBelowNegSNR,'r-',lw=3,label='Negative signal')

    # Want to plot the theoretical curve expected from Gaussian noise. 
    x=np.arange(1000)*10./1000
    y=np.zeros(x.size)
    for i in range(x.size):
        y[i] = 0.5 * snrmap.size * scipy.special.erfc(x[i]/math.sqrt(2.))
    plt.plot(x,y,'g:',label='Gaussian noise')
    
    plt.xlim(0.8,30)
    plt.ylim(1,1.e7)
    labelPlot('Signal-to-noise ratio','Number of pixels exceeding SNR','SNR pixel distribution','medium')
    plt.legend()
    plt.savefig('snrCounts.png')
    
#####################################
#  source counts

#    fin=open(sourceCatalogue)
#    sourcelist=[]
#    for line in fin:
#        if line[0]!='#':
#            sourcelist.append(models.SelavyObject(line))
#    fin.close()

    sourcecat = readCat(sourceCatalogue,sourceCatType)
    sourcelist = sourcecat.values()
   
    # list of log10(flux) points - the middle of the bins
    minFlux = inputPars.get_value('sourceCounts.minFlux',1.e-4)
    maxFlux = inputPars.get_value('sourceCounts.maxFlux',10.)
    logbinwidth=inputPars.get_value('sourceCounts.logBinWidth',0.2)
    
    logfluxpts=np.arange((math.log10(maxFlux)-math.log10(minFlux))/logbinwidth)*logbinwidth + math.log10(minFlux)
    fluxpts=10**logfluxpts
    fluxbinwidths=fluxpts*10**(logbinwidth/2.)-fluxpts/10**(logbinwidth/2.)
    counts=np.zeros(fluxpts.size)
    countsPerArea = np.zeros(fluxpts.size)
    countsMatch=np.zeros(fluxpts.size)
    countsMatchPerArea = np.zeros(fluxpts.size)

    # area of a single pixel, in deg^2
    pixelarea=abs(threshWCS.wcs.cdelt[0:2].prod())
    pixelunit=threshWCS.wcs.cunit[0].strip()
    if pixelunit == 'deg':
        pixelarea = pixelarea * (math.pi/180.)**2
    fullFieldArea = threshmap.size * pixelarea

    # get list of matches
    fin=open(matchfile)
    matchedSources=[]
    for line in fin:
        matchedSources.append(line.split()[1])
    matchedSources=np.array(matchedSources)

    for source in sourcelist:
        flux=source.flux()
        loc=int((math.log10(flux)+4+0.1)*5)
        if loc>=0 and loc<fluxpts.size:
            counts[loc] = counts[loc]+1
            sourceDetArea = pixelarea * threshmap[threshmap<source.Fpeak].size
            #sourceDetArea = fullFieldArea
            countsPerArea[loc] = countsPerArea[loc] + 1./sourceDetArea
            if (matchedSources==source.id).any():
                countsMatch[loc] = countsMatch[loc] + 1
                countsMatchPerArea[loc] = countsMatchPerArea[loc] + 1./sourceDetArea

    ##########
    # Sky model comparison
#    fin=open(skymodelCatalogue)
#    skymodellist=[]
#    for line in fin:
#        if line[0]!='#':
#            skymodellist.append(models.SelavyObject(line))
#    fin.close()
    skymodelCat = readCat(skymodelCatalogue,skymodelCatType)
    skymodellist = skymodelCat.values()
    countsSM=np.zeros(fluxpts.size)
    countsPerAreaSM = np.zeros(fluxpts.size)
    selector.setFluxType("peak")
    for source in skymodellist:
        if selector.isGood(source):
            flux=source.flux()
            loc=int((math.log10(flux)+4+0.1)*5)
            sourceDetArea = pixelarea * threshmap[threshmap<source.Fpeak].size
            #sourceDetArea = fullFieldArea
            if loc >= 0 and loc < countsSM.size and sourceDetArea > 0.:
                countsSM[loc] = countsSM[loc]+1
                countsPerAreaSM[loc] = countsPerAreaSM[loc] + 1./sourceDetArea

    #########
    # original sky model comparison, if requested
    if not skymodelOrigCat == '':
        fin=open(skymodelOrigCat)
        origskymodellist=[]
        for line in fin:
            if not line[0] == '#':
                origskymodellist.append(models.FullStokesS3SEXObject(line))
        fin.close()
        countsSMorig=np.zeros(fluxpts.size)
        countsPerAreaSMorig = np.zeros(fluxpts.size)
        selector.setFluxType("int")
        for source in origskymodellist:
            if (skymodelOrigCatIsPrecessed and selectorUnprecessed.isgood(source)) or (not skymodelOrigCatIsPrecessed and selector.isGood(source)):
                flux=source.flux()
                loc=int((math.log10(flux)+4+0.1)*5)
                #sourceDetArea = fullFieldArea
                sourceDetArea = pixelarea * threshmap[threshmap<source.flux()].size
                if loc >= 0 and loc < countsSMorig.size and sourceDetArea > 0. :
                    countsSMorig[loc] = countsSMorig[loc]+1
                    countsPerAreaSMorig[loc] = countsPerAreaSMorig[loc] + 1./sourceDetArea
        
            
    plt.figure(num=4,figsize=(8.,8.),dpi=72)
    #plt.subplot(428)
    plt.loglog()
    shift=1.
    if not skymodelOrigCat == '':
        n=countsPerAreaSMorig * fluxpts**2.5 / fluxbinwidths
        plt.plot(fluxpts*shift,n,'g-',label='_nolegend_')
        plt.plot(fluxpts*shift,n,'go',label='Original Sky model')
        plt.errorbar(fluxpts*shift,n,yerr=np.sqrt(countsSMorig)*fluxpts**2.5/fullFieldArea,xerr=None,fmt='g-',label='_nolegend_')
    n=countsPerAreaSM * fluxpts**2.5 / fluxbinwidths
    #    for i in range(len(fluxpts)):
    #        print "%d: %f %6d %f %f %d"%(i,fluxpts[i],countsSM[i],countsPerAreaSM[i],n[i],threshmap[threshmap<fluxpts[i]].size)
    plt.plot(fluxpts/shift,n,'r-',label='_nolegend_')
    plt.plot(fluxpts/shift,n,'ro',label='Sky model')
    plt.errorbar(fluxpts/shift,n,yerr=np.sqrt(countsSM)*fluxpts**2.5/fullFieldArea,xerr=None,fmt='r-',label='_nolegend_')
    n=countsPerArea * fluxpts**2.5 / fluxbinwidths
    plt.plot(fluxpts,n,'b-',label='_nolegend_')
    plt.plot(fluxpts,n,'bo',label='Detected Components')
    plt.errorbar(fluxpts,n,yerr=np.sqrt(counts)*fluxpts**2.5/fullFieldArea,xerr=None,fmt='b-',label='_nolegend_')
    n=countsMatchPerArea * fluxpts**2.5 / fluxbinwidths
    plt.plot(fluxpts,n,'g-',label='_nolegend_')
    plt.plot(fluxpts,n,'go',label='Detected & Matched Components')
    plt.errorbar(fluxpts,n,yerr=np.sqrt(countsMatch)*fluxpts**2.5/fullFieldArea,xerr=None,fmt='g-',label='_nolegend_')

    # This will plot the analytic polynomial fit to the 1.4GHz source
    # counts from Hopkins et al (2003) (AJ 125, 465)
    showHopkinsPoly=inputPars.get_value("sourceCounts.showHopkinsPolynomial",False)
    if showHopkinsPoly:
        hopkinsPolyCoeffs=[-0.008,0.057,-0.121,-0.049,0.376,0.508,0.859]
        hopkinsPoly=np.poly1d(hopkinsPolyCoeffs)
        hopkins=10**hopkinsPoly(np.log10(fluxpts*1000.))
        plt.plot(fluxpts,hopkins,'k:',label='Hopkins et al (2003)')
    
    plt.ylim(1.e-1,1.e5)
    labelPlot('S [Jy]', r'$S^{5/2}n(S)$ [ Jy$^{3/2}$ sr$^{-1}$ ]','Differential source counts','medium')
    plt.legend(loc='best')
    
    plt.savefig('sourceCounts.png')

    if not skymodelOrigCat == '':
        print "Source counts: Numbers in each bin, for Components | Matched components | Skymodel | Original sky model"
    else:
        print "Source counts: Numbers in each bin, for Components | Matched components | Skymodel "
    for i in range(len(fluxpts)):
        if not skymodelOrigCat == '':
            print "%f: %6d %6d %6d %6d"%(fluxpts[i],counts[i],countsMatch[i],countsSM[i],countsSMorig[i])
        else:
            print "%f: %6d %6d %6d"%(fluxpts[i],counts[i],countsMatch[i],countsSM[i])
            
    plt.close()
