#!/usr/bin/env python
"""
"""
import askap.analysis.evaluation
from matplotlib import *
from numpy import *
import os
import pyfits
import pywcs
from askap.analysis.evaluation.readData import *
from askap.analysis.evaluation.distributionPlotsNew import *
from askap.analysis.evaluation.distributionPlots import *
from askap.analysis.evaluation.sourceSelection import *
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

    imageName=inputPars.get_value('image','image.i.clean.taylor.0.restored.fits')
    haveBeam = os.path.exists(imageName)
    if haveBeam:
        image = pyfits.open(imageName)
        imhead = image[0].header
        bmaj = imhead.get('bmaj')*3600.
        bmin = imhead.get('bmin')*3600.
        bpa  = imhead.get('bpa')
    else:
        print "Image file %s does not exist. Not showing beam size."%imageName

    # Tool used to determine whether a given missed reference source should be included
    selector = sourceSelector(inputPars)

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
            dot='k,'
        else:
            print "Producing individual plots"
            dot='k.'
	    
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
        fluxMin = inputPars.get_value('fluxMin',-1.)
        fluxMax = inputPars.get_value('fluxMax',-1.)
	
        if doSinglePlot:
            plt.figure(num=3,figsize=(12,9),dpi=72)
            subplots_adjust(wspace=0.5,hspace=0.5)
        else:
            plt.figure(num=2,figsize=(8,8),dpi=72)
            print "Flux ratio vs flux"
            
        if doSinglePlot:
            plt.subplot(3,4,1)
	
        plt.semilogx()
        plt.plot(refFlux,fluxratio,dot)

        x=1.e-3*10**(arange(101)*3./100.)
        plt.plot(x,(x+imageNoise)/x,'k-')
        plt.plot(x,(x-imageNoise)/x,'k-')
        plt.plot(x,np.ones(x.size),'k-')
        plt.plot(x,(x+imageNoise*3)/x,'k--')
        plt.plot(x,(x-imageNoise*3)/x,'k--')
        plt.plot(x,(imageNoise*5)/x,'k:')

        themin,themax=plt.xlim()
        if fluxMin > 0:
            themin = fluxMin
        if fluxMax > 0:
            themax = fluxMax
        plt.xlim(themin,themax)
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
            print "Flux diff vs flux"
            plt.cla()
            plt.semilogx()
            plt.plot(refFlux,fluxdiff,dot)
            plt.xlim(themin,themax)
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
            print "Major axis ratio vs flux"

        plt.semilogx()
        plt.plot(refFlux,majratio,dot)
	
        plt.xlim(themin,themax)
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
            print "Major axis ratio vs major axis"

        plt.plot(refMaj,majratio,dot)
        majmin,majmax=plt.xlim()
        if haveBeam:
            plt.axvline(bmaj,color='r')
            x=np.linspace(majmin,majmax,101)
            plt.plot(x,bmaj/x,'r-')
        plt.plot(refMaj,majratio,dot)
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
            print "Flux ratio vs major axis ratio"

        plt.plot(fluxratio,majratio,dot)
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
            print "Flux diff vs major axis"
            plt.cla()
            plt.plot(refMaj,fluxdiff,dot)
            if haveBeam:
                plt.axvline(bmaj,color='r')
            plt.ylabel('Source flux - Reference flux')
            plt.xlabel('Reference Major Axis')
            plt.title(sourceCatFile)
            plt.savefig('majorAxis_fluxdiff.png')
	
	    ############################
	    # Major axis ratio vs flux diff plot        
        if not doSinglePlot:
            print "Major axis ratio vs flux diff"
            plt.cla()
            plt.plot(fluxdiff,majratio,dot)
            plt.ylim(ratioMin,ratioMax)
            plt.xlabel('Source flux - Reference flux')
            plt.ylabel('Source Major Axis / Reference Major Axis')
            plt.title(sourceCatFile)
            plt.savefig('majorAxisRatio_fluxdiff.png')
	
 	    ############################
	    # Positional Offset vs flux diff plot        
        if not doSinglePlot:
            print "Flux diff vs positional offset"
            plt.cla()
            plt.plot(dpos,fluxdiff,dot)
            plt.ylabel('Source flux - Reference flux')
            plt.xlabel('Positional offset [arcsec]')
            plt.title(sourceCatFile)
            plt.savefig('posoffset_fluxdiff.png')
	
	    
 	    ############################
	    # Positional Offset vs flux ratio plot        
        if not doSinglePlot:
            print "Flux ratio vs positional offset"
            plt.cla()
            plt.plot(dpos,fluxratio,dot)
            plt.ylabel('Source flux / Reference flux')
            plt.xlabel('Positional offset [arcsec]')
            plt.title(sourceCatFile)
            plt.savefig('posoffset_fluxratio.png')
	
	    
	    ############################
	    # Axial ratio change, vs flux
	    #  *** DO NOT INCLUDE IN THE SINGLE PLOT ***
        if not doSinglePlot:
            print "Axial ratio change vs flux"	        
            plt.cla()
            plt.semilogx()
            plt.plot(refFlux,srcAxialRatio/refAxialRatio,dot)	
            plt.xlim(themin,themax)
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
            print "Position angle change vs flux"

        plt.semilogx()
        plt.plot(refFlux,paDiff,dot)
        plt.xlim(themin,themax)
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
            print "Position angle change vs major axis"

        plt.semilogx()
        plt.plot(refMaj,paDiff,dot)
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
            print "Positional offsets"

        plt.plot(dra,ddec,dot)
        plt.axis('equal')
	    
	    #plot error ellipse
        angle=linspace(0,2*pi,100)
        plt.plot(dra.std()*cos(angle)+dra.mean(),ddec.std()*sin(angle)+ddec.mean(),'r-')
	
        if doSinglePlot:
            plt.xlabel(r'$\Delta$RA $\cos\delta$ [arcsec]')
            plt.ylabel(r'$\Delta$Dec [arcsec]')
        else:
            plt.xlabel('(Source RA - Reference RA) * cos(ref.Dec) [arcsec]')
            plt.ylabel('Source Dec - Reference Dec [arcsec]')
            plt.title(sourceCatFile)
            plt.savefig('posOffsets.png')
	
	    
        ##################################
	    # Completeness & Reliability plots

        f=[]
        for m in matchlist:
            f.append(m.ref.flux())
            f.append(m.src.flux())
        for s in srcmisslist:
            f.append(s.flux())
        for r in refmisslist:
            if selector.isGood(r):
                f.append(r.flux())
                
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
            if selector.isGood(r):
                binNumber = int((log10(r.flux())-minFlux)*10)
                numMissRefBinnedByFlux[binNumber] += 1
	
        numSrcBinnedByFlux = numMatchBinnedByFlux + numMissSrcBinnedByFlux
        numRefBinnedByFlux = numMatchBinnedByFlux + numMissRefBinnedByFlux
	
        fluxBin=10**(minFlux-0.1+arange((maxFlux-minFlux)*10+3)/10.)
	    
        completenessBinnedByFlux=np.zeros(numMatchBinnedByFlux.shape)
        completenessBinnedByFlux[numRefBinnedByFlux>0] = numMatchBinnedByFlux[numRefBinnedByFlux>0] / numRefBinnedByFlux[numRefBinnedByFlux>0]
        completenessBinnedByFlux[numRefBinnedByFlux==0] = -1
        clist=[0]
        clist.extend(completenessBinnedByFlux)
        clist.append(completenessBinnedByFlux[-1])
        clist.append(0.)
        completenessBinnedByFlux=np.array(clist)
	    
        reliabilityBinnedByFlux = np.zeros(numMatchBinnedByFlux.shape)
        reliabilityBinnedByFlux[numSrcBinnedByFlux>0] = numMatchBinnedByFlux[numSrcBinnedByFlux>0] / numSrcBinnedByFlux[numSrcBinnedByFlux>0]
        reliabilityBinnedByFlux[numSrcBinnedByFlux==0] = -1
        rlist=[0]
        rlist.extend(reliabilityBinnedByFlux)
        rlist.append(reliabilityBinnedByFlux[-1])
        rlist.append(0.)
        reliabilityBinnedByFlux=np.array(rlist)

        jointValid = (numRefBinnedByFlux>0) * (numSrcBinnedByFlux>0)
        reliabilityBinnedByFluxJoint = numMatchBinnedByFlux[jointValid] / numSrcBinnedByFlux[jointValid]
        completenessBinnedByFluxJoint = numMatchBinnedByFlux[jointValid] / numRefBinnedByFlux[jointValid]
	        
        if doSinglePlot:
            plt.subplot(3,4,9)
            crossSize=3.
        else:
            plt.cla()
            print "Completeness"
            crossSize=10.
        plt.semilogx()
        plt.axis('normal')
        plt.step(fluxBin,completenessBinnedByFlux,where='post')
        for i in range(len(fluxBin)):
            if completenessBinnedByFlux[i] < 0.:
                plt.plot(fluxBin[i]*10**0.05,-0.01,'kx',markersize=crossSize)
        plt.ylim(-0.05,1.05)
        plt.xlim(10**minFlux,10**(maxFlux+0.2))
        plt.xlabel('Flux')
        plt.ylabel('Completeness')
        if not doSinglePlot:
            plt.title(sourceCatFile)
            plt.savefig('completeness.png')
	
        if doSinglePlot:
            plt.subplot(3,4,10)
        else:
            plt.cla()
            print "Reliability"
        plt.semilogx()
        plt.step(fluxBin,reliabilityBinnedByFlux,where='post')
        for i in range(len(fluxBin)):
            if reliabilityBinnedByFlux[i] < 0.:
                plt.plot(fluxBin[i]*10**0.05,-0.01,'kx',markersize=crossSize)
        plt.ylim(-0.05,1.05)
        plt.xlim(10**(minFlux-0.2),10**(maxFlux+0.2))
        plt.xlabel('Flux')
        plt.ylabel('Reliability')
        if not doSinglePlot:
            plt.title(sourceCatFile)
            plt.savefig('reliability.png')
	
        if doSinglePlot:
            plt.subplot(3,4,8)
        else:
            plt.cla()
            print "Completeness vs Reliability"
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
        a=[]
        for m in matchlist:
            f.append(m.ref.flux())
            f.append(m.src.flux())
            a.append(m.src.maj)
            a.append(m.ref.maj)
        for s in srcmisslist:
            f.append(s.flux())
            a.append(s.maj)
        for r in refmisslist:
            if selector.isGood(r):
                f.append(r.flux())
                a.append(r.maj)

        f=np.array(f,dtype=float)
        minFlux=floor(log10(f.min())*2.)/2.
        maxFlux=ceil(log10(f.max())*2.)/2.
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
            if selector.isGood(r):
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
            print "Completeness by flux and major axis"
        extent=(minFlux,maxFlux,amin,amax)
        plt.imshow(comp2d,cmap='rainbow',interpolation='nearest',origin='lower',extent=extent)
        plt.axis('normal')
        plt.ylim(amin,amax)
        plt.xlabel('log10(Flux)')
        plt.xticks(rotation=45)
        plt.ylabel('Major axis')
        plt.title('Completeness',fontsize='small')
	    
        if doSinglePlot:
            plt.subplot(3,4,12)
        else:
            plt.cla()
            print "Reliability by flux and major axis"
        extent=(minFlux,maxFlux,amin,amax)
        plt.imshow(rel2d,cmap='rainbow',interpolation='nearest',origin='lower',extent=extent)
        plt.axis('normal')
        plt.ylim(amin,amax)
        plt.xlabel('log10(Flux)')
        plt.xticks(rotation=45)
        plt.ylabel('Major axis')
        plt.title('Reliability',fontsize='small')
	    
	    #############################
        if doSinglePlot:
            plt.suptitle(sourceCatFile,y=0.95)
            plt.savefig('finderEval.png')
    
    plt.close()
