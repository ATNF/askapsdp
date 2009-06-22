#!/usr/bin/env python

import askap.analysis.data
from numpy import *
import MySQLdb
from optparse import OptionParser
import os
import askap.analysis.data.parsets as parsets
import math

S3SEX_host = "gijane.atnf.csiro.au"
S3SEX_user = "skads"
S3SEX_pass = "skads"
S3SEX_db = "S3SEX"

SuperBrightSource = 67005861  #component number of the source with 1400MHz flux = 584Jy

def getTabName(x,y,type):
    '''
    Return a SKADS S3SEX components table name based on the position offset and the source type
    '''
    xsign=''
    if(x<0):
        xsign='m'
    elif(x>0):
        xsign='p'
    ysign=''
    if(y<0):
        ysign='m'
    elif(y>0):
        ysign='p'
    name="Components_%s_%s%d_%s%d"%(type,xsign,abs(x),ysign,abs(y))
    return name

############

if __name__ == '__main__':

    parser = OptionParser()
    parser.add_option("-i","--inputs", dest="inputfile", default="createSKADS.in", help="Input parameter file [default: %default]")

    (options, args) = parser.parse_args()

    if(not os.path.exists(options.inputfile)):
        print "Input file %s does not exist!"%options.inputfile
#        exit(1)

    inputPars = parsets.getInputParams(options.inputfile, "createSKADS.")

    nullarray = array([0.])
    if( parsets.getParamValue(inputPars, 'catSource', 'SKADS') != 'SKADS'):  # Leave open the possibility for alternative sources.
        print "Can only accept 'SKADS' as a value for 'catSource'.\nExiting.\n"
        exit(1)

    defaultTypes = array(['RQAGN','FRI','FRII','SBG','SFG'])
    types = parsets.getParamArray(inputPars, "sourceTypes", defaultTypes)
    print "Source Types requested: ",types

    fieldAngSize = float(parsets.getParamValue(inputPars, "fieldAngSize", 10.))
    fieldPixSize = int(parsets.getParamValue(inputPars, "fieldPixSize", 8192))
    fieldLocation = parsets.getParamArray(inputPars, "fieldLocation", array([187.5,-45.])).astype(float)
    pixelSize = float(parsets.getParamValue(inputPars, "pixelSize", 4.))/3600.
    
    haveFreqInfo = (parsets.getParamValue(inputPars, "haveFreqInfo", 'true').lower() == 'true')
    numChannels = int(parsets.getParamValue(inputPars, "numChannels", 16))
    centralFreq = float(parsets.getParamValue(inputPars, "centralFreq", 1.272e9))
    channelWidth = float(parsets.getParamValue(inputPars, "channelWidth", 16.e6))
    
    fluxLimit = float(parsets.getParamValue(inputPars, "fluxLimit", 1.e-5))
    
    makeImage = (parsets.getParamValue(inputPars, "makeImage", 'true').lower() == 'true')
    
    translateLoc = (parsets.getParamValue(inputPars, "translateLoc", 'true').lower() == 'true')
    oldLocation = parsets.getParamArray(inputPars, "fieldLocation", array([0., 0.])).astype(float)
    
    catFile = parsets.getParamValue(inputPars, "catFile", "catalogues/SKADS_S3SEX_10sqdeg_1uJy.dat")
    imageFile = parsets.getParamValue(inputPars, "imageFile", "images/SKADS_S3SEX_10sqdeg_1uJy.fits")
    imageParsetFile = parsets.getParamValue(inputPars, "imageParsetFile", "parsets/createFITS_SKADS.in")
    
    centres = range(-int(fieldAngSize/2),int(fieldAngSize/2)+1,1)

    db = MySQLdb.connect(host=S3SEX_host, user=S3SEX_user, passwd=S3SEX_pass, db=S3SEX_db)
    cursor = db.cursor()
    
    if(translateLoc):
        origCatFile = catFile[:catFile.rfind('.')]+'_orig'+catFile[catFile.rfind('.'):] #This is is the name of the file that has data from the database, with positions centred on the "oldLocation" position
    else:
        origCatFile = catFile  #No tranlation of positions, so we use the given filename

    catfile = file(origCatFile,"w")
    catfile.write("#%9s %10s %20s %10s %10s %10s %10s %10s\n"%("RA","Dec","Flux_1400","Alpha","Beta","Maj_axis","Min_axis","Pos_ang"))

    for type in types:
        for x in centres:
            for y in centres:
                
                tabname = getTabName(x,y,type)

                query = "SELECT right_ascension,declination,pow(10,i_1400) as flux14,pow(10,i_610) as flux6,major_axis,minor_axis,position_angle,component FROM %s WHERE i_1400>=%s"%(tabname,math.log10(fluxLimit))
                print query

                cursor.execute(query)
                results=array(cursor.fetchall())
                print shape(results)

                for r in results:

                    # Label the results values clearly so we know what we are working with!
                    ra = r[0]
                    dec = r[1]
                    s1400 = r[2]
                    s0610 = r[3]
                    maj = r[4]
                    min = r[5]
                    pa = r[6]
                    compNum = r[7]

                    if(maj>0 and min==0.):  
                        # This fixes sources where major_axis>0 yet minor_axis=0: infinite axial ratio!
                        min = 0.0001

                    if(compNum == SuperBrightSource): 
                        # This fixes the super-bright source, on the assumption that its fluxes are lacking a minus sign
                        s1400 = 1. / s1400
                        s0610 = 1. / s0610

                    alpha = log10(s1400/s0610)/log10(1400./610.)
                    beta  = 0.  #Only using two fluxes to interpolate, so can't get a curvature term.

                    catfile.write("%10.6f %10.6f %20.16f %10.6f %10.6f %10.6f %10.6f %10.6f\n"%(ra,dec,s1400,alpha,beta,maj,min,pa))

    catfile.close()

############
# Make FITS image
#   use simFITS code in askapsoft

    print "Writing to image-creating parset file %s"%imageParsetFile

    flagTranslateLoc = ('%s'%translateLoc).lower()
    flagHaveFreqInfo = ('%s'%haveFreqInfo).lower()

    createFITSinput = """\
createFITS.filename         = !%s
createFITS.sourcelist       = %s
createFITS.posType          = deg
createFITS.bunit            = JY/PIXEL
createFITS.dim              = 4
createFITS.axes             = [%d,%d,1,%d]
createFITS.WCSimage.ctype   = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
createFITS.WCSimage.cunit   = [deg, deg, "", Hz]
createFITS.WCSimage.crval   = [%5.1f, %5.1f, 1., %e]
createFITS.WCSimage.crpix   = [%d, %d, 1., %d]
createFITS.WCSimage.crota   = [0., 0., 0., 0.]
createFITS.WCSimage.cdelt   = [%f, %f, 1., %f]
createFITS.outputList       = %s
"""%(imageFile, origCatFile, fieldPixSize, fieldPixSize, numChannels, fieldLocation[0], fieldLocation[1], centralFreq, int(fieldPixSize/2)+1, int(fieldPixSize/2)+1, int(numChannels/2)+1, -pixelSize, pixelSize, channelWidth, flagTranslateLoc)
    
    if(translateLoc):
        createFITSinput += """\
createFITS.outputSourceList = %s
createFITS.WCSsources       = true
createFITS.WCSsources.ctype = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
createFITS.WCSsources.cunit = [deg, deg, "", Hz]
createFITS.WCSsources.crval = [%f,%f, 1., %f]
createFITS.WCSsources.crpix = [%d, %d, 1., %d]
createFITS.WCSsources.crota = [0., 0., 0., 0.]
createFITS.WCSsources.cdelt = [%f, %f, 1., %f]
""" % (catFile,oldLocation[0], oldLocation[1], centralFreq, int(fieldPixSize/2)+1, int(fieldPixSize/2)+1, int(numChannels/2)+1, -pixelSize,pixelSize,channelWidth)

    createFITSinput += """\
createFITS.addNoise         = false
createFITS.doConvolution    = false
createFITS.baseFreq         = 1.4e9
createFITS.flagSpectralInfo = %s
"""%(flagHaveFreqInfo)

    f = file("parsets/createFITS_SKADS.in","w")
    f.write(createFITSinput)
    f.close()

    if(makeImage):
        print "About to run createFITS to make the FITS file."
        createFITScall = "%s/Code/Components/Analysis/simulations/trunk/apps/createFITS.sh -inputs %s"%(os.environ["ASKAP_ROOT"],imageParsetFile)
        os.system(createFITScall)
