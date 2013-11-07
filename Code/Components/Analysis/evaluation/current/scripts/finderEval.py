#!/usr/bin/env python
"""
"""
import askap.analysis.evaluation
from matplotlib import *
from numpy import *
import os
from askap.analysis.evaluation.readData import *
from askap.analysis.evaluation.distributionPlotsNew import *
from askap.analysis.evaluation.distributionPlots import *
from optparse import OptionParser
import askap.parset as parset

import askap.logging

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

    matchfile = inputPars.get_value('matchfile','matches.txt')
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

    ############################
    # Flux ratio plot, with guide lines showing noise and search limit
    imageNoise = inputPars.get_value('imageNoise','')
    if imageNoise == '':
        logging.error('Eval.imageNoise not provided. Doing no evaluation.')
        exit(0)
    else:
        imageNoise=float(imageNoise)
    ratioMin = inputPars.get_value('ratioMin',0.)
    ratioMax = inputPars.get_value('ratioMax',2.5)
    fluxMin = inputPars.get_value('fluxMin',0.002)
    fluxMax = inputPars.get_value('fluxMax',1.5)

    plt.figure(num=2,figsize=(8,8),dpi=72)
    plt.semilogx()
    for m in matchlist:
        fluxratio=m.src.flux()/m.ref.flux()
        plt.plot(m.ref.flux(),fluxratio,'k.')

    x=1.e-3*10**(arange(101)*3./100)
    plt.plot(x,(x+imageNoise)/x,'k-')
    plt.plot(x,(x-imageNoise)/x,'k-')
    plt.plot(x,(x+imageNoise*3)/x,'k--')
    plt.plot(x,(x-imageNoise*3)/x,'k--')
    plt.plot(x,(imageNoise*5)/x,'k:')

    plt.xlabel('Reference flux')
    plt.ylabel('Source flux / Reference flux')
    plt.title(sourceCatFile)
    plt.xlim(fluxMin,fluxMax)
    plt.ylim(ratioMin,ratioMax)
    
    plt.savefig('fluxRatio.png')

    ############################
    # Major axis ratio vs flux plot        
    plt.cla()
    plt.semilogx()
    for m in matchlist:
        majratio=m.src.maj/m.ref.maj
        plt.plot(m.ref.flux(),majratio,'k.')

    plt.xlabel('Reference flux')
    plt.ylabel('Source Major Axis / Reference Major Axis')
    plt.title(sourceCatFile)
    plt.xlim(fluxMin,fluxMax)
    plt.ylim(ratioMin,ratioMax)
    
    plt.savefig('majorAxisRatio_flux.png')

    
    ############################
    # Major axis ratio vs major axis plot        
    plt.cla()
    for m in matchlist:
        majratio=m.src.maj/m.ref.maj
        plt.plot(m.ref.maj,majratio,'k.')

    plt.xlabel('Reference major axis')
    plt.ylabel('Source Major Axis / Reference Major Axis')
    plt.title(sourceCatFile)
    #    plt.xlim(fluxMin,fluxMax)
    plt.ylim(ratioMin,ratioMax)
    
    plt.savefig('majorAxisRatio_majorAxis.png')

    
    ############################
    # Major axis ratio vs flux ratio plot        
    plt.cla()
    for m in matchlist:
        majratio=m.src.maj/m.ref.maj
        fluxratio=m.src.flux()/m.ref.flux()
        plt.plot(fluxratio,majratio,'k.')

    plt.xlabel('Source flux / Reference flux')
    plt.ylabel('Source Major Axis / Reference Major Axis')
    plt.title(sourceCatFile)
    plt.xlim(ratioMin,ratioMax)
    plt.ylim(ratioMin,ratioMax)
    
    plt.savefig('majorAxisRatio_fluxRatio.png')

    
    ############################
    # Axial ratio change, vs flux
    plt.cla()
    for m in matchlist:
        srcAxialRatio=m.src.min/m.src.maj
        refAxialRatio=m.ref.min/m.ref.maj
        plt.plot(m.ref.flux(),srcAxialRatio/refAxialRatio,'k.')

    plt.semilogx()
    plt.xlabel('Reference flux')
    plt.ylabel('Source Axial Ratio / Reference Axial Ratio')
    plt.title(sourceCatFile)
    plt.xlim(fluxMin,fluxMax)
    plt.ylim(ratioMin,ratioMax)
    
    plt.savefig('axialRatioChange_flux.png')

    
    ############################
    # Position angle change, vs flux
    plt.cla()
    for m in matchlist:
        paDiff = m.src.pa - m.ref.pa
        plt.plot(m.ref.flux(),paDiff,'k.')

    plt.semilogx()
    plt.xlabel('Reference flux')
    plt.ylabel('Source Position Angle - Reference Position Angle [deg]')
    plt.title(sourceCatFile)
    plt.xlim(fluxMin,fluxMax)
    #    plt.ylim(ratioMin,ratioMax)
    
    plt.savefig('posangDiff_flux.png')

    ############################
    # Position angle change, vs major axis
    plt.cla()
    for m in matchlist:
        paDiff = m.src.pa - m.ref.pa
        plt.plot(m.ref.maj,paDiff,'k.')

    plt.semilogx()
    plt.xlabel('Major Axis')
    plt.ylabel('Source Position Angle - Reference Position Angle [deg]')
    plt.title(sourceCatFile)
    #    plt.xlim(fluxMin,fluxMax)
    #    plt.ylim(ratioMin,ratioMax)
    
    plt.savefig('posangDiff_majorAxis.png')

    ############################
    # Positional offsets
    plt.cla()
    for m in matchlist:
        dra = (m.src.ra - m.ref.ra)*cos(m.ref.dec*pi/180.) * 3600.
        ddec = (m.src.dec - m.ref.dec) * 3600.
        plt.plot(dra,ddec,'k.')

    plt.xlabel('Source RA - Reference RA [arcsec]')
    plt.ylabel('Source Dec - Reference Dec [arcsec]')
    plt.title(sourceCatFile)
    #    plt.xlim(fluxMin,fluxMax)
    #    plt.ylim(ratioMin,ratioMax)
    
    plt.savefig('posOffsets.png')

    
    ############################
    # Completeness plot

    f=[]
    for m in matchlist:
        f.append(m.ref.flux())
        f.append(m.src.flux())
    f=np.array(f,dtype=float)
    minFlux=floor(log10(f.min())*2.)/2.
    maxFlux=ceil(log10(f.max())*2.)/2.
    print f.min(),f.max(),minFlux,maxFlux
    
    numMatchBinnedByFlux = np.zeros((maxFlux-minFlux)*10)
    numMissSrcBinnedByFlux = np.zeros((maxFlux-minFlux)*10)
    numMissRefBinnedByFlux = np.zeros((maxFlux-minFlux)*10)
    
    for m in matchlist:
        binNumber = int((log10(m.src.flux())-minFlux)*10)
        numMatchBinnedByFlux[binNumber] += 1

    for s in srcmisslist:
        binNumber = int((log10(s.flux())-minFlux)*10)
        numMissSrcBinnedByFlux[binNumber] += 1

    for r in refmisslist:
        binNumber = int((log10(r.flux())-minFlux)*10)
        numMissRefBinnedByFlux[binNumber] += 1

    numSrcBinnedByFlux = numMatchBinnedByFlux + numMissSrcBinnedByFlux
    numRefBinnedByFlux = numMatchBinnedByFlux + numMissRefBinnedByFlux

    fluxBin=10**(minFlux+arange((maxFlux-minFlux)*10)/10.)
    
    completenessBinnedByFlux = numMatchBinnedByFlux / numRefBinnedByFlux
    compValid = np.array(1-isnan(completenessBinnedByFlux),dtype=bool)
    completenessBinnedByFluxValid=completenessBinnedByFlux[compValid]
    fluxBinComp = fluxBin[compValid]
    
    reliabilityBinnedByFlux = numMatchBinnedByFlux / numSrcBinnedByFlux
    relValid = np.array(1-isnan(reliabilityBinnedByFlux),dtype=bool)
    reliabilityBinnedByFluxValid=reliabilityBinnedByFlux[relValid]
    fluxBinRel = fluxBin[relValid]

    jointValid = compValid * relValid
    reliabilityBinnedByFluxJoint = reliabilityBinnedByFlux[jointValid]
    completenessBinnedByFluxJoint=completenessBinnedByFlux[jointValid]
        
    print completenessBinnedByFlux
    print reliabilityBinnedByFlux
    print fluxBin
    
    plt.cla()
    plt.semilogx()
    plt.step(fluxBinComp,completenessBinnedByFluxValid)
    plt.ylim(0,1.1)
    print minFlux,maxFlux
    plt.xlim(10**minFlux,10**maxFlux)
    plt.xlabel('Reference Flux')
    plt.ylabel('Completeness')
    plt.title(sourceCatFile)
    plt.savefig('completeness.png')

    plt.cla()
    plt.semilogx()
    plt.step(fluxBinRel,reliabilityBinnedByFluxValid)
    plt.ylim(0,1.1)
    plt.xlim(minFlux,maxFlux)
    plt.xlabel('Reference Flux')
    plt.ylabel('Reliability')
    plt.title(sourceCatFile)
    plt.savefig('reliability.png')

    plt.cla()
    plt.plot(reliabilityBinnedByFluxJoint,completenessBinnedByFluxJoint,'bo')
    plt.plot(reliabilityBinnedByFluxJoint,completenessBinnedByFluxJoint,'b-')
    plt.plot(reliabilityBinnedByFluxJoint[0],completenessBinnedByFluxJoint[0],'ro')
    plt.plot(reliabilityBinnedByFluxJoint[-1],completenessBinnedByFluxJoint[-1],'go')
    plt.ylim(0,1.1)
    plt.xlim(0,1.1)
    plt.xlabel('Reliability')
    plt.ylabel('Completeness')
    plt.title(sourceCatFile)
    plt.savefig('completeness_reliability.png')


    
    
    plt.close()
