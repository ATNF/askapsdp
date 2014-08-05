#!/usr/bin/env python

## @file
#  A file containing definitions of model components

from math import *
from utils import *
import pyfits
import pywcs
import os
import askap.parset as parset


class sourceSelector:

    def __init__(self, parset):

        self.method = parset.get_value('sourceSelection','none')
        self.setFluxType("peak")
        if self.method == 'threshold':
            self.threshImageName = parset.get_value('thresholdImage','detectionThreshold.i.clean.fits')
            if os.path.exists(self.threshImageName):
                threshim=pyfits.open(self.threshImageName)
                self.threshmap = threshim[0].data
                threshHeader = threshim[0].header
                self.threshWCS = pywcs.WCS(threshHeader)
                threshim.close()
                print "Using threshold map %s to determine source inclusion"%self.threshImageName
            else:
                print "Threshold image %s not available. Switching sourceSelection to 'none'."%self.threshImageName
                self.method='none'
        elif self.method == 'weights':
            self.weightsImageName = parset.get_value('weightsImage','weights.i.clean.fits')
            if os.path.exists(self.weightsImageName):
                weightsim = pyfits.open(self.weightsImageName)
                self.weightsmap = weightsim[0].data
                weightsHeader = weightsim[0].header
                self.weightsWCS = pywcs.WCS(weightsHeader)
                self.weightCutoff = parset.get_value('weightsCutoff',0.1)
                print "Using weights image %s with relative cutoff %f (=%f) to determine source inclusion"%(self.weightsImageName,self.weightCutoff,self.weightCutoff*self.weightsmap.max())
                self.weightCutoff = self.weightCutoff * self.weightsmap.max()
            else:
                print "Weights image %s not available. Switching sourceSelection to 'none'."%self.weightsImageName
                self.method='none'

    def setFluxType(self, fluxType):
        self.fluxType = fluxType
        if not (self.fluxType == "peak" or self.fluxType == "int"):
            self.fluxType = "peak"

    def setWCSreference(self, raRef, decRef):
        if self.method == 'threshold':
            threshim=pyfits.open(self.threshImageName)
            threshHeader = threshim[0].header
            threshHeader.update('CRVAL1',raRef)
            threshHeader.update('CRVAL2',decRef)
            self.threshWCS = pywcs.WCS(threshHeader)
            threshim.close()
        elif self.method == 'weights':
            weightsim=pyfits.open(self.threshImageName)
            weightsHeader = weightsim[0].header
            weightsHeader.update('CRVAL1',raRef)
            weightsHeader.update('CRVAL2',decRef)
            self.threshWCS = pywcs.WCS(weightsHeader)
            weightsim.close()
                
            
    def isGood(self,source):
        
        if self.method == 'none':
            return True
        elif self.method == 'threshold':
            skycrd=np.array([self.threshWCS.wcs.crval])
            skycrd[0][0]=source.ra
            skycrd[0][1]=source.dec
            if self.fluxType == "peak":
                flux = source.peak()
            else:
                flux = source.flux()
            useRef=False
            pixcrd=self.threshWCS.wcs_sky2pix(skycrd,1)
            if (pixcrd[0][0]>0 and pixcrd[0][0]<self.threshmap.shape[-1]) and (pixcrd[0][1]>0 and pixcrd[0][1]<self.threshmap.shape[-2]) :
                pos=tuple(np.array(pixcrd[0][::-1],dtype=int)-1)
                if self.threshmap[pos] > 0 and self.threshmap[pos] < flux :
                    useRef=True
            return useRef
        elif self.method == 'weights':
            skycrd=np.array([self.weightsWCS.wcs.crval])
            skycrd[0][0]=source.ra
            skycrd[0][1]=source.dec
            useRef=False
            pixcrd=self.weightsWCS.wcs_sky2pix(skycrd,1)
            if (pixcrd[0][0]>0 and pixcrd[0][0]<self.weightsmap.shape[-1]) and (pixcrd[0][1]>0 and pixcrd[0][1]<self.weightsmap.shape[-2]) :
                pos=tuple(np.array(pixcrd[0][::-1],dtype=int)-1)
                if self.weightsmap[pos] > self.weightCutoff :
                    useRef=True
            return useRef
            
