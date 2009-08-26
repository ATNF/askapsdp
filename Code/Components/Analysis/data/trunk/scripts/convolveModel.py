#!/usr/bin/env python2.6

import os
from optparse import OptionParser
from math import *

def getMPIxdim(numNodes):
    '''
    Return the x-dimension that fits with the number of worker nodes. Chosen to make the subdivision as square as possible.
    '''
    for i in range(sqrt(float(numNodes)),0,-1):
        if(float(numNodes)/i == trunc(float(numNodes)/i)):
            return float(numNodes)/i


parser = OptionParser()
parser.add_option("-s","--nosublists", action="store_false", dest="doSublists", default=True, help="Do not create sublists [default: %default]")
parser.add_option("-i","--noimage", action="store_false", dest="doImage", default=True, help="Do not create image [default: %default]")
parser.add_option("-a","--noanalysis", action="store_false", dest="doAnalysis", default=True, help="Do not do the analysis [default: %default]")
parser.add_option("-t","--test", action="store_true", dest="doTest", default=False, help="Just do a small test case [default: %default]")
parser.add_option("-c","--noconvolve", action="store_false", dest="doConvolve", default=True, help="Do not do the image convolution [default: %default]")
parser.add_option("-n","--nonoise", action="store_false", dest="useNoise", default=True, help="Do not add noise to the image [default: %default]")
parser.add_option("-N","--Nodes", action="store", dest="numNodes", type="int", default=1, help="Number of nodes to run analysis on, via MPI [default: %default]")
parser.add_option("-E","--EndToEnd", action="store_true", dest="useE2E", default=False, help="Use the end2end branch instead of trunk [default: %default]")
parser.add_option("-x","--nocrossmatch", action="store_false", dest="doCrossMatch", default=True, help="Do not do the cross-matching [default: %default]")
parser.add_option("-u","--noupdate", action="store_false", dest="doCatUpdate", default=True, help="Do not update the comparison catalogue and associated sublists/annotations [default: %default]")

(options, args) = parser.parse_args()

if(options.doConvolve):
    imagename = "SKADS_model_smoothed_20arcsec.fits"
    doConv = "true"
else:
    imagename = "SKADS_model_smoothed_20arcsec_noconv.fits"
    doConv = "false"
    options.doAnalysis = False

if(options.useNoise):
    useNoise = "true"
else:
    useNoise = "false"

if(options.doCrossMatch):
    analysisApp = "continuumAnalysis.sh"
else:
    analysisApp = "cduchamp.sh"

useMPI=False
nsubx=1
nsuby=1
if(options.numNodes>1):
    numWorkers=options.numNodes-1
    nsubx = getMPIxdim(numWorkers)
    nsuby = numWorkers/nsubx
    useMPI=True


if(options.useE2E):
#pathToSims = "%s/Code/Components/Analysis/simulations/tags/simulations-end2end1/bin"%os.environ['ASKAP_ROOT']
#pathToAnalysis =  "%s/Code/Components/Analysis/analysis/tags/analysis-end2end1/bin"%os.environ['ASKAP_ROOT']
    pathToSims = "%s/Code/Components/Analysis/simulations/tags/simulations-end2end1/apps"%os.environ['ASKAP_ROOT']
    pathToAnalysis =  "%s/Code/Components/Analysis/analysis/tags/analysis-end2end1/apps"%os.environ['ASKAP_ROOT']
    pathToData =  "%s/Code/Components/Analysis/data/tags/data-end2end1/install/bin"%os.environ['ASKAP_ROOT']
    pathToCat = "%s/Code/Components/Analysis/data/tags/data-end2end1/catalogues/"%os.environ['ASKAP_ROOT']
else:
#pathToSims = "%s/Code/Components/Analysis/simulations/trunk/install/bin"%os.environ['ASKAP_ROOT']
#pathToAnalysis =  "%s/Code/Components/Analysis/analysis/trunk/install/bin"%os.environ['ASKAP_ROOT']
    pathToSims = "%s/Code/Components/Analysis/simulations/trunk/apps"%os.environ['ASKAP_ROOT']
    pathToAnalysis =  "%s/Code/Components/Analysis/analysis/trunk/apps"%os.environ['ASKAP_ROOT']
    pathToData =  "%s/Code/Components/Analysis/data/trunk/install/bin"%os.environ['ASKAP_ROOT']
    pathToCat = "%s/Code/Components/Analysis/data/trunk/catalogues"%os.environ['ASKAP_ROOT']

cwd = os.getcwd()

bmaj=sqrt(2.*20**2)
bmin=bmaj
bpa=0.
parsetFullTest="""\
## The next block creates sublists of the main catalogue for use with the testing.
#
createSubs.catfilename      = %s/SKADS_S3SEX_10sqdeg_1uJy.dat
createSubs.flagAnnotation   = true
createSubs.thresholds       = [0,10,200]
createSubs.radii            = [0.5,5]
createSubs.destDir          = .
#
## The next block creates a model to match the image out of the end-to-end simulation
createFITS.filename         = !%s
#createFITS.sourcelist       = %s/SKADS_S3SEX_10sqdeg_1uJy.dat
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
#
imageQual.image             = %s
imageQual.srcFile           = %s/duchamp-Summary.txt
imageQual.refFile           = %s/SKADS_S3SEX_10sqdeg_1uJy_10uJy_5.0deg.txt
imageQual.refPosType        = deg
imageQual.RA                = 12:30:00
imageQual.Dec               = -45:00:00
imageQual.fluxUseFit        = yes
#imageQual.epsilon          = 10.
#imageQual.fluxMethod       = integrated
"""%(pathToCat,imagename,pathToCat,useNoise,doConv,bmaj/3600.,bmin/3600.,bpa,imagename,nsubx,nsuby,useNoise,imagename,cwd,cwd)

## The next three assume a 20arcsec beam
bmaj=20
bmin=bmaj
bpa=0.
## The next three are assuming we add 20 arcsec from weiner & restoring in parallel
#bmaj=sqrt(2.*20**2)
#bmin=bmaj
#bpa=0.
## The next three are using the measured (fitted) shape of the psf.image.i.skads
#bmaj=38.532
#bmin=37.653
#bpa=93.98
parsetSmallTest="""\
## The next block creates sublists of the main catalogue for use with the testing.
#
createSubs.catfilename      = %s/SKADS_S3SEX_10sqdeg_1uJy.dat
createSubs.flagAnnotation   = true
createSubs.thresholds       = [0,200,1000]
createSubs.radii            = [0.5]
createSubs.destDir          = .
#
# The next block creates a small image in the centre for testing purposes
createFITS.filename         = !SKADS_model_smoothed_20arcsec_test.fits
createFITS.sourcelist       = SKADS_S3SEX_10sqdeg_1uJy_1000uJy_0.5deg.txt
#createFITS.sourcelist       = SKADS_S3SEX_10sqdeg_1uJy_200uJy_0.5deg.txt
createFITS.posType          = deg
createFITS.bunit            = Jy/beam
createFITS.dim              = 4
createFITS.axes             = [1024,1024,1,1]
createFITS.WCSimage.ctype   = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
createFITS.WCSimage.cunit   = [deg, deg, "", Hz]
createFITS.WCSimage.crval   = [187.5, -45.0, 1., 1.272000e+09]
createFITS.WCSimage.crpix   = [513,513, 1., 1]
createFITS.WCSimage.crota   = [0., 0., 0., 0.]
createFITS.WCSimage.cdelt   = [-0.00138889, 0.00138889, 1., 16000000.000000]
createFITS.outputList       = false
createFITS.doConvolution    = true
createFITS.beam             = [%8.6f,%8.6f,%6.4f]
createFITS.baseFreq         = 1.4e9
createFITS.flagSpectralInfo = false
createFITS.PAunits          = rad
createFITS.minMinorAxis     = 0.000100
createFITS.noiserms         = 1.e-8
createFITS.addNoise         = %s
#
## The next block controls the source-detection & fitting & cross-matching.
#
Cduchamp.image              = %s/SKADS_model_smoothed_20arcsec_test.fits
Cduchamp.nsubx              = %d
Cduchamp.nsuby              = %d
Cduchamp.overlapx           = 50
Cduchamp.overlapy           = 50
Cduchamp.threshold          = 5.e-6
Cduchamp.doFit              = true
#Cduchamp.Fitter.fitTypes    = [psf]
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
#
imageQual.image             = %s/SKADS_model_smoothed_20arcsec_test.fits
imageQual.srcFile           = %s/duchamp-Summary.txt
imageQual.refFile           = %s/SKADS_S3SEX_10sqdeg_1uJy_1000uJy_0.5deg.txt
#imageQual.refFile           = %s/200uJy_comp_list.txt
#imageQual.refPosType        = deg
imageQual.refPosType        = dms
imageQual.RA                = 12:30:00
imageQual.Dec               = -45:00:00
#imageQual.fluxUseFit        = yes
imageQual.epsilon          = 10.
imageQual.fluxMethod       = integrated
"""%(pathToCat,bmaj/3600.,bmin/3600.,bpa,useNoise,cwd,nsubx,nsuby,useNoise,cwd,cwd,cwd,cwd)


parsetfileFull = "model_testing.in"
f = file(parsetfileFull,"w")
f.write(parsetFullTest)
f.close()
parsetfileTest = "model_testing_sml.in"
f = file(parsetfileTest,"w")
f.write(parsetSmallTest)
f.close()

if(options.doTest):
    parsetfile = parsetfileTest
else:
    parsetfile = parsetfileFull

if(options.doSublists):
    print "##\nCreating the sublists\n##\n"
    os.system(pathToData+"/createSubLists.py -i %s"%parsetfile)
if(options.doImage):
    print "##\nCreating the image\n##\n"
    os.system(pathToSims+"/createFITS.sh -inputs %s"%parsetfile)
if(options.doAnalysis):
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

mpirun -np %d %s/%s -inputs %s/%s 1>& %s/analysis.log
"""%(1+options.numNodes/4,options.numNodes,pathToAnalysis,analysisApp,cwd,parsetfile,cwd)
            f = file("model_testing.qsub","w")
            f.write(qsubfile)
            f.close()
            os.system("qsub model_testing.qsub")
            print "Have submitted the job -- check the queue in the usual manner for completion."
        else:
            os.system("mpirun -np %d %s/%s -inputs %s 1>& analysis.log"%(options.numNodes,pathToAnalysis,analysisApp,parsetfile))
    else:
        os.system(pathToAnalysis+"/%s -inputs %s 1>& analysis.log"%(analysisApp,parsetfile))


if(options.doCatUpdate):

    print "##\nCreating the new catalogue\n##\n"
    newCatName = "newCatalogue_200uJy_20arcsec.txt"
    awkStatement="tail -`wc -l duchamp-Summary.txt | awk '{print $1-2}'` duchamp-Summary.txt | awk '{if($16>0) printf \"%%s  %%s  %%11.8f  %%8.3f  %%8.3f  %%6.4f\\n\",$2,$3,$6,$8,$9,$10*3.14159265359/180.}' > %s"%newCatName
    print "##\nExecuting statement: %s\n##\n"%awkStatement
    os.system(awkStatement)
    sublistParset="""\
createSubs.catfilename      = %s
createSubs.flagAnnotation   = true
createSubs.thresholds       = [0]
createSubs.radii            = [3.5]
createSubs.destDir          = .
"""%newCatName
    parsetfile = "model_testing_subs.in"
    f = file(parsetfile,"w")
    f.write(sublistParset)
    f.close()
    print "##\nCreating the sublists of the new catalogue\n##\n"
    os.system(pathToData+"/createSubLists.py -i %s"%parsetfile)

####
