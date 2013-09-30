#!/usr/bin/env python

import askap.analysis.evaluation

import numpy as np
import pylab as plt
from math import * 
import askap.analysis.evaluation.modelcomponents as models
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
    sourceCatalogue=inputPars.get_value('sourceCatalogue','selavy-fitResults.txt')

    
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
    threshWCS = pywcs.WCS(threshim[0].header)
    threshim.close()
    
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
    logSNRlevel=np.arange(5*log10(20.)+1)/5.
    snrLevel=10**logSNRlevel
    for snr in snrLevel:
        numPixAboveSNR.append(snrmap[snrmap>snr].size)
        numPixBelowNegSNR.append(snrmap[snrmap<-snr].size)

    numPixAboveSNR=np.array(numPixAboveSNR,dtype=float)
    numPixBelowNegSNR=np.array(numPixBelowNegSNR,dtype=float)
    x=np.arange(1000)*10./1000
    y=snrmap.size*np.exp(-x*x/2.)

    plt.figure(num=3,figsize=(8,8),dpi=72)
    #plt.subplot(427)
    plt.loglog()
    plt.plot(snrLevel,numPixAboveSNR,'b-',lw=3,label='Positive signal')
    plt.plot(snrLevel,numPixBelowNegSNR,'r-',lw=3,label='Negative signal')
    plt.plot(x,y,'g:',label='Gaussian noise')
    plt.xlim(0.8,30)
    plt.ylim(1,1.e7)
    labelPlot('Signal-to-noise ratio','Number of pixels exceeding SNR','SNR pixel distribution','medium')
    plt.legend()
    plt.savefig('snrCounts.png')
    
#####################################
#  source counts

    fin=open(sourceCatalogue)
    sourcelist=[]
    for line in fin:
        if line[0]!='#':
            sourcelist.append(models.SelavyObject(line))
    fin.close()

# list of log10(flux) points - the middle of the bins
    logfluxpts=np.arange((log10(10)-log10(1.e-4))*5)/5. - 4.
    fluxpts=10**logfluxpts
    counts=np.zeros(fluxpts.size)
    countsPerArea = np.zeros(fluxpts.size)

# area of a single pixel, in deg^2
    pixelarea=abs(threshWCS.wcs.cdelt[0:2].prod())
    fullFieldArea = threshmap.size * pixelarea

    for source in sourcelist:
        flux=source.FintFIT
        loc=int((log10(flux)+4+0.1)*5)
        counts[loc] = counts[loc]+1
        sourceDetArea = pixelarea * threshmap[threshmap<source.Fpeak].size
        #sourceDetArea = fullFieldArea
        countsPerArea[loc] = countsPerArea[loc] + 1./sourceDetArea

##########
# Sky model comparison
    fin=open(skymodelCatalogue)
    skymodellist=[]
    for line in fin:
        if line[0]!='#':
            skymodellist.append(models.SelavyObject(line))
    fin.close()
    countsSM=np.zeros(fluxpts.size)
    countsPerAreaSM = np.zeros(fluxpts.size)
    skycrd=np.array([threshWCS.wcs.crval])
    for source in skymodellist:
        skycrd[0][0]=source.ra
        skycrd[0][1]=source.dec
        pixcrd=threshWCS.wcs_sky2pix(skycrd,1)
        if( (pixcrd[0][0]>0 and pixcrd[0][0]<threshmapFull.shape[-1]) and (pixcrd[0][1]>0 and pixcrd[0][1]<threshmapFull.shape[-2]) ):
            flux=source.FintFIT
            loc=int((log10(flux)+4+0.1)*5)
            countsSM[loc] = countsSM[loc]+1
            sourceDetArea = pixelarea * threshmap[threshmap<source.Fpeak].size
            #sourceDetArea = fullFieldArea
            countsPerAreaSM[loc] = countsPerAreaSM[loc] + 1./sourceDetArea

    plt.figure(num=4,figsize=(8.,8.),dpi=72)
    #plt.subplot(428)
    plt.loglog()
    n=countsPerAreaSM * fluxpts**2.5
    plt.plot(fluxpts,n,'ro',label='Sky model')
    plt.errorbar(fluxpts,n,yerr=np.sqrt(countsSM)*fluxpts**2.5/fullFieldArea,xerr=None,label='_nolegend_')
    n=countsPerArea * fluxpts**2.5
    plt.plot(fluxpts,n,'bo',label='Component list')
    plt.errorbar(fluxpts,n,yerr=np.sqrt(counts)*fluxpts**2.5/fullFieldArea,xerr=None,label='_nolegend_')
    labelPlot('S [Jy]', r'$S^{5/2}n(S)$ [ Jy$^{3/2}$ sr$^{-1}$ ]','Differential source counts','medium')
    plt.legend(loc='best')
    
    plt.savefig('sourceCounts.png')


    plt.close()
