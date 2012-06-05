#!/usr/bin/env python
"""
"""
import askap.analysis.evaluation
from matplotlib import *
from numpy import *
import os
from askap.analysis.evaluation.readData import *
from askap.analysis.evaluation.distributionPlots import *
from optparse import OptionParser
import askap.parset as parset

if __name__ == '__main__':

    parser = OptionParser()
    parser.add_option("-i","--inputs", dest="inputfile", default="", help="Input parameter file [default: %default]")

    (options, args) = parser.parse_args()

    if(options.inputfile==''):
        inputPars = parset.ParameterSet()        
    elif(not os.path.exists(options.inputfile)):
        print "Input file %s does not exist!\nUsing default parameter values."%options.inputfile
        inputPars = parset.ParameterSet()
    else:
        inputPars = parset.ParameterSet(options.inputfile).plotEval

    matchfile = inputPars.get_value('matchfile',"matches.txt")
    missfile = inputPars.get_value('missfile',"misses.txt")

    if(not os.access(matchfile,os.F_OK)):
        print "Match file %s does not exist. Doing no evaluation"%matchfile
        exit(0)

    if(not os.access(missfile,os.F_OK)):
        print "Miss file %s does not exist. Doing no evaluation"%missfile
        exit(0)

    plotRefMisses = inputPars.get_value('plotRefMisses',False)

    matchType,idS,xS,yS,fS,aS,bS,pS,chisq,imagerms,rms,nfree,ndof,npf,npo,idR,xR,yR,fR,aR,bR,pR = read_match_data(matchfile)
    missType,id,x,y,f,chisq2,imagerms2,rms2,nfree2,ndof2,npf2,npo2 = read_miss_data(missfile)

    if(size(x)>0):
        print "Match list size = %d, Miss list size = %d (%d source and %d reference)"%(size(xS),size(x),size(missType[missType=='S']),size(missType[missType=='R']))
    else:
        print "Match list size = %d, Miss list size = %d"%(size(xS),size(x))

#    figure(1, figsize=(16.5,11.7), dpi=72)
    figure(1, figsize=(11.7,11.7), dpi=72)

    subplots_adjust(wspace=0.3,hspace=0.3)

    posOffsetPlot(xS,yS,xR,yR,matchType)
    axisrange=spatPosPlot(xS,yS,xR,yR,matchType,x,y,missType,minRelVal=2.,plotRefMisses=plotRefMisses)

    xrms=append(xS,x[npo2>0])
    yrms=append(yS,y[npo2>0])
    irms=append(imagerms,imagerms2[npo2>0])
    type=append(matchType,missType[npo2>0])
#    rmsSpatPlot(xS,yS,imagerms,matchType,axisrange)
    rmsSpatPlot(xrms,yrms,irms,type,axisrange)
    
#    spatHistPlot(fS,fR,xS,yS, axisrange, minRelVal=0., scaleByRel=True, absoluteSizes=False, removeZeros=True, name='S', unit='\mu Jy', locationCode=233, plotTitle='Flux difference')
#    spatHistPlot(aS,aR,xS,yS, axisrange, minRelVal=0., scaleByRel=True, absoluteSizes=True, removeZeros=True, name='A', unit='\prime\prime', locationCode=234, plotTitle='Major axis difference')
#    spatHistPlot(bS,bR,xS,yS, axisrange, minRelVal=0., scaleByRel=True, absoluteSizes=True, removeZeros=True, name='B', unit='\prime\prime', locationCode=235, plotTitle='Minor axis difference')
#    PAspatHistPlot(aS,pS,pR,xS,yS, axisrange, minRelVal=1., removeZeros=True, locationCode=236)
    spatHistPlot(fS,fR,xS,yS, axisrange, minRelVal=0., scaleByRel=False, absoluteSizes=True, sizeStep=100., removeZeros=True, name='S', unit='\mu Jy', locationCode=334, plotTitle='Flux difference')
    spatHistPlot(fS,fR,xS,yS, axisrange, minRelVal=0., scaleByRel=True, absoluteSizes=False, removeZeros=True, name='S', unit='\mu Jy', locationCode=335, plotTitle='Rel. Flux difference')

#    subplot(336)
#    meandx = mean(xS-xR)
#    meandy = mean(yS-yR)
#    offset = sqrt( (xS-xR-meandx)**2 + (yS-yR-meandy)**2 )
#    plot(offset,imagerms,'o')

    spatHistPlot(aS,aR,xS,yS, axisrange, minRelVal=0., scaleByRel=False, absoluteSizes=True, sizeStep=5., removeZeros=True, name='A', unit='\prime\prime', locationCode=337, plotTitle='Major axis difference')
    spatHistPlot(bS,bR,xS,yS, axisrange, minRelVal=0., scaleByRel=False, absoluteSizes=True, sizeStep=5., removeZeros=True, name='B', unit='\prime\prime', locationCode=338, plotTitle='Minor axis difference')
    PAspatHistPlot(aS,pS,pR,xS,yS, axisrange, minRelVal=1., removeZeros=True, locationCode=339)

    savefig('plotEval')

#    show()
    close()

