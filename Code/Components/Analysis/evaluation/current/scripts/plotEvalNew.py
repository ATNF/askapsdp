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
        logging.error("Eval.sourceCatalogue %s does not exist. Doing no evaluation."%matchfile)
        exit(0)
    sourceCatType = inputPars.get_value('sourceCatalogueType','Selavy')
    sourceCat = readCat(sourceCatFile,sourceCatType)
    
    refCatFile = inputPars.get_value('refCatalogue','')
    if(refCatFile == ''):
        logging.error('Eval.refCatalogue not provided. Doing no evaluation.')
        exit(0)
    if(not os.access(refCatFile,os.F_OK)):
        logging.error("Eval.refCatalogue %s does not exist. Doing no evaluation."%matchfile)
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

    plotRefMisses = inputPars.get_value('plotRefMisses',False)

    figure(1, figsize=(11.7,11.7), dpi=72)
    subplots_adjust(wspace=0.3,hspace=0.3)

    posOffsetPlotNew(matchlist)

    # Put data into individual arrays to match old plotting functions.
    xS=array([])
    yS=array([])
    xR=array([])
    yR=array([])
    matchType=array([])
    imagerms=array([])
    fS=array([])
    fR=array([])
    aS=array([])
    bS=array([])
    pS=array([])
    aR=array([])
    bR=array([])
    pR=array([])
    for m in matchlist:
        xS=append(xS,m.src.ra)
        yS=append(yS,m.src.dec)
        xR=append(xR,m.ref.ra)
        yR=append(yR,m.ref.dec)
        matchType=append(matchType,m.type)
        imagerms=append(imagerms,m.src.RMSimage)
        fS=append(fS,m.src.flux())
        fR=append(fS,m.ref.flux())
        aS=append(aS,m.src.maj)
        bS=append(bS,m.src.min)
        pS=append(pS,m.src.pa)
        aR=append(aR,m.ref.maj)
        bR=append(bR,m.ref.min)
        pR=append(pR,m.ref.pa)
        
    x=array([])
    y=array([])
    missType=array([])
    npo2=array([])
    imagerms2=array([])
    for m in srcmisslist:
        x=append(x,m.ra)
        y=append(y,m.dec)
        missType=append(missType,'S')
        npo2=append(npo2,sourceCat[m.id].npixObj)
        imagerms2=append(imagerms2,sourceCat[m.id].RMSimage)
    for m in refmisslist:
        x=append(x,m.ra)
        y=append(y,m.dec)
        missType=append(missType,'R')
        npo2=append(npo2,refCat[m.id].npixObj)
        imagerms2=append(imagerms2,refCat[m.id].RMSimage)
                         
    axisrange=spatPosPlot(xS,yS,xR,yR,matchType,x,y,missType,minRelVal=2.,plotRefMisses=plotRefMisses)

    xrms=append(xS,x[npo2>0])
    yrms=append(yS,y[npo2>0])
    irms=append(imagerms,imagerms2[npo2>0])
    type=append(matchType,missType[npo2>0])
    rmsSpatPlot(xrms,yrms,irms,type,axisrange)
    
    spatHistPlot(fS,fR,xS,yS, axisrange, minRelVal=0., scaleByRel=False, absoluteSizes=True, sizeStep=100., removeZeros=True, name='S', unit='\mu Jy', locationCode=334, plotTitle='Flux difference')
    spatHistPlot(fS,fR,xS,yS, axisrange, minRelVal=0., scaleByRel=True, absoluteSizes=False, removeZeros=True, name='S', unit='\mu Jy', locationCode=335, plotTitle='Rel. Flux difference')

    spatHistPlot(aS,aR,xS,yS, axisrange, minRelVal=0., scaleByRel=False, absoluteSizes=True, sizeStep=5., removeZeros=True, name='A', unit='\prime\prime', locationCode=337, plotTitle='Major axis difference')
    spatHistPlot(bS,bR,xS,yS, axisrange, minRelVal=0., scaleByRel=False, absoluteSizes=True, sizeStep=5., removeZeros=True, name='B', unit='\prime\prime', locationCode=338, plotTitle='Minor axis difference')
    PAspatHistPlot(aS,pS,pR,xS,yS, axisrange, minRelVal=1., removeZeros=True, locationCode=339)


    savefig('plotEval')

    close()
