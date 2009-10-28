#!/usr/bin/env python

import askap.analysis.data
import os
from optparse import OptionParser
import askap.parset as parset
from math import *
from numpy import *

from askap import logging

logger = logging.getLogger(__name__)


############

if __name__ == '__main__':

    parser = OptionParser()
    parser.add_option("-i","--inputs", dest="inputfile", default="", help="Input parameter file [default: %default]")

    (options, args) = parser.parse_args()

    if(options.inputfile==''):
        print "No parset given: using default values for all parameters."
        inputPars = parset.ParameterSet()
    elif(not os.path.exists(options.inputfile)):
        print "Input file %s does not exist!\nUsing default parameter values."%options.inputfile
        inputPars = parset.ParameterSet()
    else:
        print "Using parset %s to obtain parameters."%options.inputfile
        inputPars = parset.ParameterSet(options.inputfile).compCat

    doConvolution = inputPars.get_value("doConvolution",True);
    modelImage = inputPars.get_value("modelImage","");
    convolvedImage = inputPars.get_value("convolvedImage","");
    major = inputPars.get_value("major",30.)
    minor = inputPars.get_value("minor",30.)
    pa = inputPars.get_value("pa",0.)

    doCduchamp = inputPars.get_value("doCduchamp",True);
    useMPI = inputPars.get_value("useMPI",False);
    numNodes = inputPars.get_value("numNodes", 1);

    inputPars.set_value('Cduchamp.image',convolvedImage)
    if('Cduchamp.Fitter.useNoise' not in inputPars):
        inputPars.set_value('Cduchamp.Fitter.useNoise',False)
    if('Cduchamp.Fitter.fitTypes' not in inputPars):
        inputPars.set_value('Cduchamp.Fitter.fitTypes','[full]')
    if('Cduchamp.Fitter.maxRMS' not in inputPars):
        inputPars.set_value('Cduchamp.Fitter.maxRMS','500.')
    if('Cduchamp.Fitter.boxPadSize' not in inputPars):
        inputPars.set_value('Cduchamp.Fitter.boxPadSize','7')
    if('Cduchamp.threshSpatial' not in inputPars):
        inputPars.set_value('Cduchamp.threshSpatial','10')


    cduchampSummary = inputPars.get_value("Cduchamp.summaryFile","duchamp-Summary.txt")
    outputCat = inputPars.get_value("outputCat","");

    sublistParsetFile = "createCompCat-sublists.in"

    if(doConvolution):
        
        script="""
ia.open('%s')
ia.convolve2d(outfile='%s',major='%f arcsec',minor='%f arcsec',pa='%f deg')
ia.close()
"""%(modelImage,convolvedImage,major,minor,pa)
        scriptFileName='createCompCat-casapy-script.py'
        scriptFile = file(scriptFileName,"w");
        scriptFile.write(script)
        scriptFile.close()

        if(os.path.exists(convolvedImage)):
            print "Convolved image %s exists - not re-creating."%convolvedImage
        else:
            os.system("casapy < %s"%scriptFileName);

###

    if(doCduchamp):

        cduchampParFileName = 'createCompCat-cduchamp.in'
        cduchampParFile = file(cduchampParFileName,"w")
        if('Cduchamp' in inputPars):
            cduchampParFile.write("%s"%inputPars.Cduchamp)
        cduchampParFile.close()

        pathToAnalysis = "%s/Code/Components/Analysis/analysis/trunk/install/bin/"%os.environ['ASKAP_ROOT']

        if(useMPI):
            if(os.uname()[1].split('.')[0]=='minicp'):
                qsubfile = """\
#!/bin/bash -l
#PBS -l nodes=%d:ppn=4
#PBS -l walltime=72:00:00
#PBS -M Matthew.Whiting@csiro.au
module load openmpi
cd $PBS_O_WORKDIR

ulimit -c unlimited

mpirun -np %d %s/cduchamp.sh -inputs %s/%s 1>& %s/analysis.log

%s/Code/Components/Analysis/data/trunk/install/bin/createSubLists.py -i %s
"""%(1+numNodes/4,numNodes,pathToAnalysis,os.getcwd(),cduchampParFileName,os.getcwd(),os.environ['ASKAP_ROOT'],sublistParsetFile)
                f = file("model_testing.qsub","w")
                f.write(qsubfile)
                f.close()
                os.system("qsub model_testing.qsub")
                print "Have submitted the job -- check the queue in the usual manner for completion."
            else:
                os.system("mpirun -np %d %s/%s -inputs %s 1>& analysis.log"%(numNodes,pathToAnalysis,analysisApp,cduchampParFileName))
        else:
            os.system(pathToAnalysis+"/%s -inputs %s 1>& analysis.log"%(analysisApp,cduchampParFileName))

#        os.system("%s/Code/Components/Analysis/analysis/trunk/install/bin/cduchamp.sh -inputs %s"%(os.environ['ASKAP_ROOT'],cduchampParFileName))

###

    inputFile = file(cduchampSummary,"r")
    outputFile = file(outputCat,"w");
    
    for line in inputFile:

        if(line[0]!='#'):

            values = line[:-1].split()
            print values
            print values[9]
            npixFit = values[15]
            if(npixFit>0):  # This means the source has a valid fit
                ra = values[1]
                dec = values[2]
                flux = float(values[5])
                maj = float(values[7])
                min = float(values[8])
                pa = float(values[9])*math.pi/180.
                outputFile.write("%s  %s  %11.8f  %8.3f  %8.3f  %6.4f\n"%(ra,dec,flux,maj,min,pa))

    outputFile.close()
    inputFile.close()

    sublistParset="""\
createSubs.catfilename      = %s
createSubs.flagAnnotation   = true
createSubs.thresholds       = [0]
createSubs.radii            = [3.5]
createSubs.destDir          = .
"""%outputCat
    f = file(sublistParsetFile,"w")
    f.write(sublistParset)
    f.close()
    if(!useMPI or os.uname()[1].split('.')[0]=='minicp'):
        os.system("%s/Code/Components/Analysis/data/trunk/install/bin/createSubLists.py -i %s"%(os.environ['ASKAP_ROOT'],sublistParsetFile))




