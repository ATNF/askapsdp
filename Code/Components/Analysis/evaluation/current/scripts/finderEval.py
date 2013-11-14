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

    plotType = inputPars.get_value('plotType','all')
    if plotType == 'all':
        plotTypeArray = [True,False]
    elif plotType == 'single':
        plotTypeArray = [True]
    elif plotType == 'individual':
        plotTypeArray = [False]
    else:
        plotTypeArray = [True,False]

    ############################
    #  Arrays needed for plotting
	
    refFlux=[]
    refMaj=[]
    fluxratio=[]
    fluxdiff=[]
    majratio=[]
    srcAxialRatio=[]
    refAxialRatio=[]
    paDiff=[]
    dra=[]
    ddec=[]
    for m in matchlist:
        refFlux.append(m.ref.flux())
        refMaj.append(m.ref.maj)
        fluxratio.append(m.src.flux()/m.ref.flux())
        fluxdiff.append(m.src.flux()-m.ref.flux())
        majratio.append(m.src.maj/m.ref.maj)
        srcAxialRatio.append(m.src.min/m.src.maj)
        refAxialRatio.append(m.ref.min/m.ref.maj)
        paDiff.append(m.src.pa - m.ref.pa)
        dra.append((m.src.ra - m.ref.ra)*cos(m.ref.dec*pi/180.) * 3600.)
        ddec.append((m.src.dec - m.ref.dec) * 3600.)

    refFlux=np.array(refFlux,dtype=float)
    refMaj=np.array(refMaj,dtype=float)
    fluxratio=np.array(fluxratio,dtype=float)
    fluxdiff=np.array(fluxdiff,dtype=float)
    majratio =np.array(majratio,dtype=float)
    srcAxialRatio =np.array(srcAxialRatio,dtype=float)
    refAxialRatio =np.array(refAxialRatio,dtype=float)
    paDiff=np.array(paDiff,dtype=float)
    dra=np.array(dra,dtype=float)
    ddec=np.array(ddec,dtype=float)    
    dpos = np.sqrt(dra**2 + ddec**2)
	

    for doSinglePlot in plotTypeArray:

        if doSinglePlot:
            print "Producing a single plot"
        else:
            print "Producing individual plots"
	    
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
            plt.figure(num=3,figsize=(12,9),dpi=72)
            subplots_adjust(wspace=0.3,hspace=0.3)
        else:
            plt.figure(num=2,figsize=(8,8),dpi=72)
            
        if doSinglePlot:
            plt.subplot(3,4,1)
	
        plt.semilogx()
	    #    for m in matchlist:
	    #    fluxratio=m.src.flux()/m.ref.flux()
	    #    plt.plot(m.ref.flux(),fluxratio,'k.')
        plt.plot(refFlux,fluxratio,'k.')

        x=1.e-3*10**(arange(101)*3./100.)
        plt.plot(x,(x+imageNoise)/x,'k-')
        plt.plot(x,(x-imageNoise)/x,'k-')
        plt.plot(x,np.ones(x.size),'k-')
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
		# Flux difference vs reference flux

        # Only for individual plots
        if not doSinglePlot:
            plt.cla()
            plt.semilogx()
            plt.plot(refFlux,fluxdiff,'k.')
            plt.xlim(fluxMin,fluxMax)
            plt.xlabel('Reference flux')
            plt.ylabel('Source flux - Reference flux')
            plt.title(sourceCatFile)
            plt.savefig('fluxDiff.png')
            
	    ############################
	    # Major axis ratio vs flux plot
        if doSinglePlot:
            plt.subplot(3,4,2)
        else:
            plt.cla()

        plt.semilogx()
        plt.plot(refFlux,majratio,'k.')
	
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

        plt.plot(refMaj,majratio,'k.')
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
	    # Major axis vs flux diff plot        
        if not doSinglePlot:
            plt.cla()
            plt.plot(refMaj,fluxdiff,'k.')
            plt.ylabel('Source flux - Reference flux')
            plt.xlabel('Reference Major Axis')
            plt.title(sourceCatFile)
            plt.savefig('majorAxis_fluxdiff.png')
	
	    ############################
	    # Major axis ratio vs flux diff plot        
        if not doSinglePlot:
            plt.cla()
            plt.plot(fluxdiff,majratio,'k.')
            plt.ylim(ratioMin,ratioMax)
            plt.xlabel('Source flux - Reference flux')
            plt.ylabel('Source Major Axis / Reference Major Axis')
            plt.title(sourceCatFile)
            plt.savefig('majorAxisRatio_fluxdiff.png')
	
 	    ############################
	    # Positional Offset vs flux diff plot        
        if not doSinglePlot:
            plt.cla()
            plt.plot(dpos,fluxdiff,'k.')
            plt.ylabel('Source flux - Reference flux')
            plt.xlabel('Positional offset [arcsec]')
            plt.title(sourceCatFile)
            plt.savefig('posoffset_fluxdiff.png')
	
	    
	    ############################
	    # Axial ratio change, vs flux
	    #  *** DO NOT INCLUDE IN THE SINGLE PLOT ***
        if not doSinglePlot:
	        
            plt.cla()
            plt.semilogx()
            plt.plot(refFlux,srcAxialRatio/refAxialRatio,'k.')	
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

        plt.semilogx()
        plt.plot(refFlux,paDiff,'k.')
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

        plt.semilogx()
        plt.plot(refMaj,paDiff,'k.')
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

        plt.plot(dra,ddec,'k.')
	    
	    #plot error ellipse
        angle=linspace(0,2*pi,100)
        plt.plot(dra.std()*cos(angle)+dra.mean(),ddec.std()*sin(angle)+ddec.mean(),'r-')
	
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
	    
        compValid = numRefBinnedByFlux>0
        completenessBinnedByFluxValid = numMatchBinnedByFlux[compValid] / numRefBinnedByFlux[compValid]
        fluxBinComp = fluxBin[compValid]
	    
        relValid = numSrcBinnedByFlux > 0
        reliabilityBinnedByFluxValid = numMatchBinnedByFlux[relValid] / numSrcBinnedByFlux[relValid]
        fluxBinRel = fluxBin[relValid]
	
        jointValid = compValid * relValid
        reliabilityBinnedByFluxJoint = numMatchBinnedByFlux[jointValid] / numSrcBinnedByFlux[jointValid]
        completenessBinnedByFluxJoint = numMatchBinnedByFlux[jointValid] / numRefBinnedByFlux[jointValid]
	        
        if doSinglePlot:
            plt.subplot(3,4,9)
        else:
            plt.cla()
        plt.semilogx()
        plt.step(fluxBinComp,completenessBinnedByFluxValid)
        plt.ylim(0,1.1)
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

        comp2d = np.zeros(nmatch2d.shape)
        rel2d = np.zeros(nmatch2d.shape)
        comp2d[nRef2d>0] = nmatch2d[nRef2d>0] / nRef2d[nRef2d>0]
        comp2d[nRef2d==0] = nan
        rel2d[nSrc2d>0]  = nmatch2d[nSrc2d>0] / nSrc2d[nSrc2d>0]
        rel2d[nSrc2d==0] = nan
	
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
