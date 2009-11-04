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

def getMPIxdim(numNodes):
    '''
    Return the x-dimension that fits with the number of worker nodes. Chosen to make the subdivision as square as possible.
    '''
    for i in range(sqrt(float(numNodes)),0,-1):
        if(float(numNodes)/i == trunc(float(numNodes)/i)):
            return float(numNodes)/i


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
        inputPars = parset.ParameterSet(options.inputfile).convolveModel

    doSublists = inputPars.get_value("doSublists", True)
    makeImage = inputPars.get_value("makeImage", True)
    doAnalysis = inputPars.get_value("doAnalysis", True)
    addNoise = inputPars.get_value("addNoise", True)
    testMode = inputPars.get_value("testMode", False)
    doConvolution = inputPars.get_value("doConvolution",True)
    numNodes = inputPars.get_value("numNodes",1)
    useE2E = inputPars.get_value("useE2E",False);
    doCrossMatch = inputPars.get_value("doCrossMatch", False)
    updateModel = inputPars.get_value("updateModel", False)

    catalogueFile = inputPars.get_value("catalogueFile","")
    newCatFile = inputPars.get_value("newCatFile","")

    arcsecPerDegree = array([3600.,3600.,1.])
    beam = array(parset.decode(inputPars.get_value("beam", "[20.,20.,0.]"))) / arcsecPerDegree

    workingDir = inputPars.get_value("workingDir",".");
    imageName = inputPars.get_value("imageName", "");
    imageSize = array(parset.decode(inputPars.get_value("imageSize","[5120,5120,1,1]")))
    crpix = imageSize/2 + 1
    pixSize = array(parset.decode(inputPars.get_value("pixSize","[5.,5.]")))
    
    strConv = ("%s"%doConvolution).lower()
    strNoise = ("%s"%addNoise).lower()

    if(doConvolution):
        imagename = "SKADS_model_smoothed_20arcsec.fits"
    else:
        imagename = "SKADS_model_smoothed_20arcsec_noconv.fits"
        doAnalysis = False
        

    useMPI=False
    nsubx=1
    nsuby=1
    if(numNodes>1):
        numWorkers=numNodes-1
        nsubx = getMPIxdim(numWorkers)
        nsuby = numWorkers/nsubx
        useMPI=True

    if(useE2E):
        pathToSims = "%s/Code/Components/Analysis/simulations/tags/simulations-end2end1/install/bin"%os.environ['ASKAP_ROOT']
        pathToAnalysis =  "%s/Code/Components/Analysis/analysis/tags/analysis-end2end1/install/bin"%os.environ['ASKAP_ROOT']
        pathToData =  "%s/Code/Components/Analysis/data/tags/data-end2end1/install/bin"%os.environ['ASKAP_ROOT']
        pathToCat = "%s/Code/Components/Analysis/data/tags/data-end2end1/catalogues/"%os.environ['ASKAP_ROOT']
    else:
        pathToSims = "%s/Code/Components/Analysis/simulations/trunk/install/bin"%os.environ['ASKAP_ROOT']
        pathToAnalysis =  "%s/Code/Components/Analysis/analysis/trunk/install/bin"%os.environ['ASKAP_ROOT']
        pathToData =  "%s/Code/Components/Analysis/data/trunk/install/bin"%os.environ['ASKAP_ROOT']
        pathToCat = "%s/Code/Components/Analysis/data/trunk/catalogues"%os.environ['ASKAP_ROOT']
        
    cwd = os.getcwd()

    parsetOut="""\
## The next block creates sublists of the main catalogue for use with the testing.
#
createSubs.catfilename      = %s
createSubs.flagAnnotation   = true
createSubs.thresholds       = [0,10,200]
createSubs.radii            = [0.5,5]
createSubs.destDir          = .
#
## The next block creates a model to match the image out of the end-to-end simulation
createFITS.filename         = !%s
createFITS.sourcelist       = SKADS_S3SEX_10sqdeg_1uJy_10uJy_5.0deg.txt
createFITS.posType          = deg
createFITS.bunit            = Jy/beam
createFITS.dim              = 4
createFITS.axes             = [5120,5120,1,1]
createFITS.WCSimage.ctype   = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
createFITS.WCSimage.cunit   = [deg, deg, "", Hz]
createFITS.WCSimage.crval   = [187.5, -45.0, 1., 1.272000e+09]
createFITS.WCSimage.crpix   = [2561,2561, 1., 1]
createFITS.WCSimage.crota   = [0., 0., 0., 0.]
createFITS.WCSimage.cdelt   = [-0.00138889, 0.00138889, 1., 16000000.000000]
createFITS.outputList       = false
createFITS.addNoise         = %s
createFITS.doConvolution    = %s
createFITS.beam             = [%8.6f, %8.6f, %6.4f]
createFITS.baseFreq         = 1.4e9
createFITS.flagSpectralInfo = false
createFITS.PAunits          = rad
createFITS.minMinorAxis     = 0.000100
#
## The next block controls the source-detection & fitting & cross-matching.
#
Cduchamp.image              = %s
Cduchamp.nsubx              = %d
Cduchamp.nsuby              = %d
Cduchamp.overlapx           = 50
Cduchamp.overlapy           = 50
Cduchamp.threshold          = 200.e-6
Cduchamp.doFit              = true
Cduchamp.Fitter.fitTypes    = [full]
Cduchamp.Fitter.boxPadSize  = 7
Cduchamp.Fitter.maxNumGauss = 5
Cduchamp.Fitter.useNoise    = %s
Cduchamp.threshSpatial      = 10
Cduchamp.flagAdjacent       = false
Cduchamp.flagGrowth         = false
Cduchamp.growthCut          = 5
Cduchamp.pixelCentre        = peak
Cduchamp.subimageAnnotationFile = duchamp-subimages.ann
"""%(catalogueFile,imagename,strNoise,strConv,beam[0],beam[1],beam[2],imagename,nsubx,nsuby,strNoise)


    parsetfile = "model_testing.in"
    f = file(parsetfile,"w")
    f.write(parsetOut)
    f.close()
    exit(0)

    if(doSublists):
        print "##\nCreating the sublists\n##\n"
        os.system(pathToData+"/createSubLists.py -i %s"%parsetfile)
    if(makeImage):
        print "##\nCreating the image\n##\n"
        os.system(pathToSims+"/createFITS.sh -inputs %s"%parsetfile)
    if(doAnalysis):
        print "##\nDoing the analysis\n##\n"
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
"""%(1+numNodes/4,numNodes,pathToAnalysis,cwd,parsetfile,cwd)
                f = file("model_testing.qsub","w")
                f.write(qsubfile)
                f.close()
                os.system("qsub model_testing.qsub")
                print "Have submitted the job -- check the queue in the usual manner for completion."
            else:
                os.system("mpirun -np %d %s/%s -inputs %s 1>& analysis.log"%(numNodes,pathToAnalysis,analysisApp,parsetfile))
        else:
            os.system(pathToAnalysis+"/%s -inputs %s 1>& analysis.log"%(analysisApp,parsetfile))


    if(updateModel):

        print "##\nCreating the new catalogue\n##\n"
        awkStatement="tail -`wc -l duchamp-Summary.txt | awk '{print $1-2}'` duchamp-Summary.txt | awk '{if($16>0) printf \"%%s  %%s  %%11.8f  %%8.3f  %%8.3f  %%6.4f\\n\",$2,$3,$6,$8,$9,$10*3.14159265359/180.}' > %s"%newCatFile
        print "##\nExecuting statement: %s\n##\n"%awkStatement
        os.system(awkStatement)
        sublistParset="""\
createSubs.catfilename      = %s
createSubs.flagAnnotation   = true
createSubs.thresholds       = [0]
createSubs.radii            = [3.5]
createSubs.destDir          = .
"""%newCatFile
        parsetfile = "model_testing_subs.in"
        f = file(parsetfile,"w")
        f.write(sublistParset)
        f.close()
        print "##\nCreating the sublists of the new catalogue\n##\n"
        os.system(pathToData+"/createSubLists.py -i %s"%parsetfile)

####
