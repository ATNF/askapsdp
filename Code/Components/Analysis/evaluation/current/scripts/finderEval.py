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

    doSinglePlot = inputPars.get_value('doSingleFinderPlot',False)

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

    if doSinglePlot:
        plt.figure(num=2,figsize=(12,9),dpi=72)
        subplots_adjust(wspace=0.3,hspace=0.3)
    else:
        plt.figure(num=2,figsize=(8,8),dpi=72)

    if doSinglePlot:
        plt.subplot(3,4,1)

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

    plt.xlim(fluxMin,fluxMax)
    plt.ylim(ratioMin,ratioMax)

    if doSinglePlot:
        plt.xlabel('Flux')
        plt.ylabel('Ratio(Flux)')
    else:
        plt.xlabel('Reference flux')
        plt.ylabel('Source flux / Reference flux')
        plt.title(sourceCatFile)
        plt.savefig('fluxRatio.png')

    ############################
    # Major axis ratio vs flux plot        
    if doSinglePlot:
        plt.subplot(3,4,2)
    else:
        plt.cla()
    plt.semilogx()
    for m in matchlist:
        majratio=m.src.maj/m.ref.maj
        plt.plot(m.ref.flux(),majratio,'k.')

    plt.xlim(fluxMin,fluxMax)
    plt.ylim(ratioMin,ratioMax)
    if doSinglePlot:
        plt.xlabel('Flux')
        plt.ylabel('Ratio(Major Axis)')
    else:
        plt.xlabel('Reference flux')
        plt.ylabel('Source Major Axis / Reference Major Axis')
        plt.title(sourceCatFile)
        plt.savefig('majorAxisRatio_flux.png')

    
    ############################
    # Major axis ratio vs major axis plot        
    if doSinglePlot:
        plt.subplot(3,4,3)
    else:
        plt.cla()
    for m in matchlist:
        majratio=m.src.maj/m.ref.maj
        plt.plot(m.ref.maj,majratio,'k.')

    #    plt.xlim(fluxMin,fluxMax)
    plt.ylim(ratioMin,ratioMax)
    if doSinglePlot:
        plt.xlabel('Major axis')
        plt.ylabel('Ratio(Major Axis)')
    else:
        plt.xlabel('Reference major axis')
        plt.ylabel('Source Major Axis / Reference Major Axis')
        plt.title(sourceCatFile)
        plt.savefig('majorAxisRatio_majorAxis.png')

    
    ############################
    # Major axis ratio vs flux ratio plot        
    if doSinglePlot:
        plt.subplot(3,4,4)
    else:
        plt.cla()
    for m in matchlist:
        majratio=m.src.maj/m.ref.maj
        fluxratio=m.src.flux()/m.ref.flux()
        plt.plot(fluxratio,majratio,'k.')

    plt.xlim(ratioMin,ratioMax)
    plt.ylim(ratioMin,ratioMax)
    if doSinglePlot:
        plt.xlabel('Ratio(Flux)')
        plt.ylabel('Ratio(Major Axis)')
    else:
        plt.xlabel('Source flux / Reference flux')
        plt.ylabel('Source Major Axis / Reference Major Axis')
        plt.title(sourceCatFile)
        plt.savefig('majorAxisRatio_fluxRatio.png')

    
    ############################
    # Axial ratio change, vs flux
    #  *** DO NOT INCLUDE IN THE SINGLE PLOT ***
    if not doSinglePlot:
        
        plt.cla()
        for m in matchlist:
            srcAxialRatio=m.src.min/m.src.maj
            refAxialRatio=m.ref.min/m.ref.maj
            plt.plot(m.ref.flux(),srcAxialRatio/refAxialRatio,'k.')

        plt.semilogx()
        plt.xlim(fluxMin,fluxMax)
        plt.ylim(ratioMin,ratioMax)
        plt.xlabel('Reference flux')
        plt.ylabel('Source Axial Ratio / Reference Axial Ratio')
    
        plt.title(sourceCatFile)
        plt.savefig('axialRatioChange_flux.png')

    
    ############################
    # Position angle change, vs flux
    if doSinglePlot:
        plt.subplot(3,4,6)
    else:
        plt.cla()
    for m in matchlist:
        paDiff = m.src.pa - m.ref.pa
        plt.plot(m.ref.flux(),paDiff,'k.')

    plt.semilogx()
    plt.xlim(fluxMin,fluxMax)
    #    plt.ylim(ratioMin,ratioMax)
    if doSinglePlot:
        plt.xlabel('Reference flux')
        plt.ylabel('Diff(Position Angle)')
    else:
        plt.xlabel('Reference flux')
        plt.ylabel('Source Position Angle - Reference Position Angle [deg]')
        plt.title(sourceCatFile)
        plt.savefig('posangDiff_flux.png')

    ############################
    # Position angle change, vs major axis
    if doSinglePlot:
        plt.subplot(3,4,7)
    else:
        plt.cla()
    for m in matchlist:
        paDiff = m.src.pa - m.ref.pa
        plt.plot(m.ref.maj,paDiff,'k.')

    plt.semilogx()
    #    plt.xlim(fluxMin,fluxMax)
    #    plt.ylim(ratioMin,ratioMax)
    if doSinglePlot:
        plt.xlabel('Major Axis')
        plt.ylabel('Diff(Position Angle)')
    else:
        plt.xlabel('Major Axis')
        plt.ylabel('Source Position Angle - Reference Position Angle [deg]')
        plt.title(sourceCatFile)
        plt.savefig('posangDiff_majorAxis.png')

    ############################
    # Positional offsets
    if doSinglePlot:
        plt.subplot(3,4,5)
    else:
        plt.cla()
    dra=[]
    ddec=[]
    for m in matchlist:
        dra.append((m.src.ra - m.ref.ra)*cos(m.ref.dec*pi/180.) * 3600.)
        ddec.append((m.src.dec - m.ref.dec) * 3600.)

    dra=np.array(dra,dtype=float)
    ddec=np.array(ddec,dtype=float)    
    plt.plot(dra,ddec,'k.')
    
    #plot error ellipse
    angle=linspace(0,2*pi,100)
    plt.plot(dra.std()*cos(angle)+dra.mean(),ddec.std()*sin(angle)+ddec.mean(),'r-')

    #    plt.xlim(fluxMin,fluxMax)
    #    plt.ylim(ratioMin,ratioMax)
    if doSinglePlot:
        plt.xlabel(r'$\delta$RA [arcsec]')
        plt.ylabel(r'$\delta$Dec [arcsec]')
    else:
        plt.xlabel('Source RA - Reference RA [arcsec]')
        plt.ylabel('Source Dec - Reference Dec [arcsec]')
        plt.title(sourceCatFile)
        plt.savefig('posOffsets.png')

    
    ############################
    # Completeness plot

    f=[]
    for m in matchlist:
        f.append(m.ref.flux())
        f.append(m.src.flux())
    for r in refmisslist:
        f.append(r.flux())
    for s in srcmisslist:
        f.append(s.flux())
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
    
    if doSinglePlot:
        plt.subplot(3,4,9)
    else:
        plt.cla()
    plt.semilogx()
    plt.step(fluxBinComp,completenessBinnedByFluxValid)
    plt.ylim(0,1.1)
    print minFlux,maxFlux
    plt.xlim(10**minFlux,10**maxFlux)
    plt.xlabel('Flux')
    plt.ylabel('Completeness')
    if not doSinglePlot:
        plt.title(sourceCatFile)
        plt.savefig('completeness.png')

    if doSinglePlot:
        plt.subplot(3,4,10)
    else:
        plt.cla()
    plt.semilogx()
    plt.step(fluxBinRel,reliabilityBinnedByFluxValid)
    plt.ylim(0,1.1)
    plt.xlim(10**minFlux,10**maxFlux)
    plt.xlabel('Flux')
    plt.ylabel('Reliability')
    if not doSinglePlot:
        plt.title(sourceCatFile)
        plt.savefig('reliability.png')

    if doSinglePlot:
        plt.subplot(3,4,8)
    else:
        plt.cla()
    plt.plot(reliabilityBinnedByFluxJoint,completenessBinnedByFluxJoint,'bo')
    plt.plot(reliabilityBinnedByFluxJoint,completenessBinnedByFluxJoint,'b-')
    plt.plot(reliabilityBinnedByFluxJoint[0],completenessBinnedByFluxJoint[0],'ro')
    plt.plot(reliabilityBinnedByFluxJoint[-1],completenessBinnedByFluxJoint[-1],'go')
    plt.ylim(0,1.1)
    plt.xlim(0,1.1)
    plt.xlabel('Reliability')
    plt.ylabel('Completeness')
    if not doSinglePlot:
        plt.title(sourceCatFile)
        plt.savefig('completeness_reliability.png')

    #############################

    f=[]
    for m in matchlist:
        f.append(m.ref.flux())
        f.append(m.src.flux())
    for r in refmisslist:
        f.append(r.flux())
    for s in srcmisslist:
        f.append(s.flux())
    f=np.array(f,dtype=float)
    minFlux=floor(log10(f.min())*2.)/2.
    maxFlux=ceil(log10(f.max())*2.)/2.
    a=[]
    for m in matchlist:
        a.append(m.src.maj)
        a.append(m.ref.maj)
    for r in refmisslist:
        a.append(r.maj)
    for s in srcmisslist:
        a.append(s.maj)
    a=np.array(a,dtype=float)
    amin=floor(a.min()/5.)*5
    amax=ceil(a.max()/5.)*5

    nmatch2d=np.zeros(((amax-amin)/5.,(maxFlux-minFlux)*10.))
    nmissSrc2d=np.zeros(((amax-amin)/5.,(maxFlux-minFlux)*10.))
    nmissRef2d=np.zeros(((amax-amin)/5.,(maxFlux-minFlux)*10.))

    for m in matchlist:
        abin=int((m.ref.maj-amin)/5.)
        fbin=int((log10(m.src.flux())-minFlux)*10)
        nmatch2d[abin][fbin] += 1
    for s in srcmisslist:
        abin=int((s.maj-amin)/5.)
        fbin=int((log10(s.flux())-minFlux)*10)
        nmissSrc2d[abin][fbin] += 1
    for r in refmisslist:
        abin=int((r.maj-amin)/5.)
        fbin=int((log10(r.flux())-minFlux)*10)
        nmissRef2d[abin][fbin] += 1
    
    nSrc2d = nmatch2d + nmissSrc2d
    nRef2d = nmatch2d + nmissRef2d

#    # Do the inverse to make the plotting better.
#    comp2d = 1-nmatch2d/nRef2d
#    rel2d  = 1-nmatch2d/nSrc2d
    comp2d = nmatch2d/nRef2d
    rel2d  = nmatch2d/nSrc2d
#
#    for x in range(comp2d.shape[0]):
#        for y in range(comp2d.shape[1]):
#            if isnan(comp2d[x][y]):
#                comp2d[x][y]=-1.
#    for x in range(rel2d.shape[0]):
#        for y in range(rel2d.shape[1]):
#            if isnan(rel2d[x][y]):
#                rel2d[x][y]=-1.

    if doSinglePlot:
        plt.subplot(3,4,11)
    else:
        plt.cla()
    extent=(minFlux,maxFlux,amin,amax)
    plt.imshow(comp2d,cmap='rainbow',interpolation='nearest',origin='lower',extent=extent)
    plt.axis('normal')
    plt.ylim(amin,amax)
    plt.xlabel('log10(Flux)')
    plt.ylabel('Major axis')
    plt.title('Completeness')
    
    if doSinglePlot:
        plt.subplot(3,4,12)
    else:
        plt.cla()
    extent=(minFlux,maxFlux,amin,amax)
    plt.imshow(rel2d,cmap='rainbow',interpolation='nearest',origin='lower',extent=extent)
    plt.axis('normal')
    plt.ylim(amin,amax)
    plt.xlabel('log10(Flux)')
    plt.ylabel('Major axis')
    plt.title('Reliability')
    
    #############################
    if doSinglePlot:
        plt.suptitle(sourceCatFile,y=0.95)
        plt.savefig('finderEval.png')
    
    plt.close()
