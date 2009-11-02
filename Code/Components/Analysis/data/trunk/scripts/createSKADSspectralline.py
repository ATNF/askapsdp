#!/usr/bin/env python

import askap.analysis.data
from numpy import *
import MySQLdb
from optparse import OptionParser
import os
import askap.parset as parset
import math

SKADS_host = "gijane.atnf.csiro.au"
SKADS_user = "skads"
SKADS_pass = "skads"
#SKADS_db = "S3SEX"

SuperBrightSource = 67005861  # component number of the source with 1400MHz flux = 584Jy
HIfreq = 1420405751.786       # emission frequency of HI line

def getTabNames(x,y,type):
    '''
    Return a SKADS S3SEX Components and Galaxies table names based on the position offset and the source type
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
    names=[]
    names.append("Components_%s_%s%d_%s%d"%(type,xsign,abs(x),ysign,abs(y)))
    names.append("Galaxies_%s_%s%d_%s%d"%(type,xsign,abs(x),ysign,abs(y)))
    return names

############

def sphericalDistance(ra1, dec1, ra2, dec2):
      r1 = ra1  * math.pi / 180.;
      d1 = dec1 * math.pi / 180.;
      r2 = ra2  * math.pi / 180.;
      d2 = dec2 * math.pi / 180.;
      angsep = math.cos(r1-r2)*math.cos(d1)*math.cos(d2) + math.sin(d1)*math.sin(d2);
      return math.acos(angsep)*180./math.pi;

############

def getQueryStringS3SEX(tabnames,max_redshift,min_redshift,fluxLimit,hiLimit=0):
    '''
    Return a query string appropriate for the database requested. The names of the relevant tables for the S3SEX case are provided by the parameter "tabnames". Basic parameters to return from the query are: RA, Dec, z, M_HI, fluxes (if available), major axis, minor axis, position angle.
    '''

    if(fluxLimit<=0.):
        fluxLimit=1.e-18;

    hiQuery = 'g.m_hi>%f and g.redshift < %f and g.redshift > %f '%(hiLimit,max_redshift,min_redshift)
    
    return "SELECT g.right_ascension,g.declination,g.redshift,g.m_hi,pow(10,g.itot_1400) as flux14,pow(10,g.itot_610) as flux6,c.right_ascension,c.declination,c.major_axis,c.minor_axis,c.position_angle,g.galaxy,c.component from %s as g left outer join %s as c on g.galaxy=c.galaxy where c.i_1400>%f and %s"%(tabnames[1],tabnames[0],log10(fluxLimit),hiQuery)

############

def getQueryStringS3SAX(radius,max_redshift,min_redshift,hiMassLimit=0.,hiFluxLimit=0.):
    '''
    Return a query string appropriate for the database requested. The names of the relevant tables for the S3SEX case are provided by the parameter "tabnames". Basic parameters to return from the query are: RA, Dec, z, M_HI, fluxes (if available), major axis, minor axis, position angle.
    '''

    sepQuery = 'acos(  cos( right_ascension * pi() / 180. ) * cos( declination * pi() / 180. ) * cos( 0. )\
 + sin( declination * pi() / 180. ) * sin( 0. )  ) * 180. / pi() <  %f and '%radius


    hiQuery = 'himass>%f and hiintflux>%f and zapparent < %f and zapparent > %f'%(hiMassLimit,hiFluxLimit,max_redshift,min_redshift)

    return 'SELECT right_ascension,declination,zapparent,himass,hiintflux,himajoraxis_msunpc as "major axis", hiaxisratio*himajoraxis_msunpc as "minor axis", diskpositionangle,hilumcenter,hilumpeak,hiwidthpeak,hiwidth50,hiwidth20 from Galaxies where %s%s'%(sepQuery,hiQuery)

############

def writeHeaderLine(catfile, database):

    if(database=='S3SEX'):
        catfile.write("#%9s %10s %20s %10s %10s %10s %10s %10s %10s %10s %5s\n"%("RA","Dec","Flux_1400","Alpha","Beta","Maj_axis","Min_axis","Pos_ang","Redshift","M_HI","Type"))
    elif(database=='S3SAX'):
        catfile.write("#%9s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s\n"%("RA","Dec","HI_Flux","Alpha","Beta","Maj_axis","Min_axis","Pos_ang","Redshift","M_HI","F_0", "Fpeak", "Wpeak", "W50", "W20"))

############

def writeResults(catfile, results, database, haveFreqInfo):

    for r in results:
        
        # Label the results values clearly so we know what we are working with!
        ra = r[0]
        dec = r[1]
        z = r[2]
        
        alpha = 0.
        beta = 0.

        if(database=='S3SEX'):
            mHI = pow(10,r[3])
            s1400 = r[4]
            s0610 = r[5]
            maj = r[8]
            min = r[9]
            pa = r[10]
            compNum = r[12]
            if(compNum == SuperBrightSource): 
                # This fixes the super-bright source, on the assumption that its fluxes are lacking a minus sign
                s1400 = 1. / s1400
                s0610 = 1. / s0610
            if(haveFreqInfo):
                alpha = log10(s1400/s0610)/log10(1400./610.)
        elif(database=='S3SAX'):
            mHI = r[3]
            intflux=r[4]
            maj = r[5]
            min = r[6]
            pa = r[7]
            f0 = r[8]
            fpeak = r[9]
            wpeak = r[10]
            w50 = r[11]
            w20 = r[12]
            
        if(database=='S3SEX'):
            catfile.write("%10.6f %10.6f %20.16f %10.6f %10.6f %10.6f %10.6f %10.6f %10.6f %10.6f %5d\n"%(ra,dec,s1400,alpha,beta,maj,min,pa,z,mHI,defaultTypes[type]))
        elif(database=='S3SAX'):
            catfile.write("%10.6f %10.6f %10.6f %10.6f %10.6f %10.6f %10.6f %10.6f %10.6f %10.6f %10.6f %10.6f %10.6f %10.6f %10.6f\n"%(ra,dec,intflux,alpha,beta,maj,min,pa,z,mHI,f0,fpeak,wpeak,w50,w20))


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
        inputPars = parset.ParameterSet(options.inputfile).createSKADS

    catSource = inputPars.get_value('catSource','SKADS')
    if( catSource != 'SKADS'):  # Leave open the possibility for alternative sources.
        print "Can only accept 'SKADS' as a value for 'catSource'.\nExiting.\n"
        exit(1)

    database = inputPars.get_value('database', 'S3SAX')
    print 'Using database %s'%database

    makeImage = inputPars.get_value("makeImage", True)
    queryDatabase = inputPars.get_value("queryDatabase",True)

    defaultTypes = {'RQAGN':0,'FRI':1,'FRII':2,'SBG':3,'SFG':4}
    types = inputPars.get_value("sourceTypes", defaultTypes.keys())
    if(database=='S3SEX'):
        print "Source Types requested: ",types

    fieldAngSize = inputPars.get_value("fieldAngSize", 5.)
    fieldPixSize = inputPars.get_value("fieldPixSize", 2048)
    fieldLocation = inputPars.get_value("fieldLocation", [187.5,-45.])
    pixelSize = inputPars.get_value("pixelSize", 2.)/3600.
    
    haveFreqInfo = inputPars.get_value("haveFreqInfo", True)
    numChannels = inputPars.get_value("numChannels", 2048)
    centralFreq = inputPars.get_value("centralFreq", 1.40226e9)
    channelWidth = inputPars.get_value("channelWidth", 18.3e3)
    imageDoContinuum = inputPars.get_value("imageDoContinuum", False)
    imageDoHI = inputPars.get_value("imageDoHI", True)

    fluxLimit = inputPars.get_value("fluxLimit", 1.e-6) # in Jy
    hiMassLimit = inputPars.get_value("hiMassLimit",1.e6) # in M_sun
    hiFluxLimit = inputPars.get_value("hiFluxLimit",1.e-3) # in Jy.km/s
    minMinorAxis = inputPars.get_value("minMinorAxis", 0.0001)
    
    translateLoc = inputPars.get_value("translateLoc", True)
    oldLocation = inputPars.get_value("oldLocation", [0., 0.])
    
    catFile = inputPars.get_value("catFile", "catalogues/SKADS_S3SEX_10sqdeg_spectralline.dat")
    imageFile = inputPars.get_value("imageFile", "images/SKADS_S3SEX_10sqdeg_spectralline.fits")
    imageParsetFile = inputPars.get_value("imageParsetFile", "parsets/createFITS_SKADSspectralline.in")
    
    if(translateLoc):
        if(catFile.rfind('.')<0):
            origCatFile = catFile + '_orig'
        else:
            origCatFile = catFile[:catFile.rfind('.')]+'_orig'+catFile[catFile.rfind('.'):] #This is is the name of the file that has data from the database, with positions centred on the "oldLocation" position
    else:
        origCatFile = catFile  #No tranlation of positions, so we use the given filename


    if(queryDatabase):
        db = MySQLdb.connect(host=SKADS_host, user=SKADS_user, passwd=SKADS_pass, db=database)
        cursor = db.cursor()
    
        catfile = file(origCatFile,"w")
        writeHeaderLine(catfile,database)

        centres = range(-int(fieldAngSize/2),int(fieldAngSize/2)+1,1)
        if(database=='S3SAX'):
            types=['SFG']
            haveFreqInfo = False;
            centres = [0]

        for type in types:
            for x in centres:
                for y in centres:

                    tabnames = getTabNames(x,y,type)

                    max_redshift = HIfreq / (centralFreq - numChannels*channelWidth/2.) - 1.
                    min_redshift = HIfreq / (centralFreq + numChannels*channelWidth/2.) - 1.

                    if(database=='S3SEX'):
                        query = getQueryStringS3SEX(tabnames,max_redshift,min_redshift,fluxLimit)
                    elif(database=='S3SAX'):
                        query = getQueryStringS3SAX(fieldAngSize,max_redshift,min_redshift,hiMassLimit,hiFluxLimit)
                    print query

                    cursor.execute(query)
                    results=array(cursor.fetchall())
                    print shape(results)
                    
                    writeResults(catfile, results, database, haveFreqInfo)

        catfile.close()

############
# Make FITS image
#   use simFITS code in askapsoft

    print "Writing to image-creating parset file %s"%imageParsetFile

    flagTranslateLoc = ('%s'%translateLoc).lower()
    flagHaveFreqInfo = ('%s'%haveFreqInfo).lower()
    flagDoContinuum = ('%s'%imageDoContinuum).lower()
    flagDoHI = ('%s'%imageDoHI).lower()

    createFITSinput = """\
createFITS.filename         = !%s
createFITS.sourcelist       = %s
createFITS.sourcelisttype   = spectralline
createFITS.database         = %s
createFITS.posType          = deg
createFITS.bunit            = Jy/pixel
createFITS.dim              = 4
createFITS.axes             = [%d,%d,1,%d]
createFITS.WCSimage.ctype   = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
createFITS.WCSimage.cunit   = [deg, deg, "", Hz]
createFITS.WCSimage.crval   = [%5.1f, %5.1f, 1., %e]
createFITS.WCSimage.crpix   = [%d, %d, 1., %d]
createFITS.WCSimage.crota   = [0., 0., 0., 0.]
createFITS.WCSimage.cdelt   = [%f, %f, 1., %f]
createFITS.outputList       = %s
"""%(imageFile, origCatFile, database, fieldPixSize, fieldPixSize, numChannels, fieldLocation[0], fieldLocation[1], centralFreq, int(fieldPixSize/2)+1, int(fieldPixSize/2)+1, int(numChannels/2)+1, -pixelSize, pixelSize, channelWidth, flagTranslateLoc)
    
    if(translateLoc):
        createFITSinput += """\
createFITS.outputSourceList = %s
createFITS.WCSsources       = true
createFITS.WCSsources.ctype = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
createFITS.WCSsources.cunit = [deg, deg, "", Hz]
createFITS.WCSsources.crval = [%f,%f, 1., %e]
createFITS.WCSsources.crpix = [%d, %d, 1., %d]
createFITS.WCSsources.crota = [0., 0., 0., 0.]
createFITS.WCSsources.cdelt = [%f, %f, 1., %f]
""" % (catFile,oldLocation[0], oldLocation[1], centralFreq, int(fieldPixSize/2)+1, int(fieldPixSize/2)+1, int(numChannels/2)+1, -pixelSize,pixelSize,channelWidth)

    createFITSinput += """\
createFITS.addNoise         = false
createFITS.doConvolution    = false
createFITS.baseFreq         = %e
createFITS.flagSpectralInfo = %s
createFITS.doContinuum      = %s
createFITS.doHI             = %s
createFITS.PAunits          = rad
createFITS.minMinorAxis     = %f
"""%(centralFreq,flagHaveFreqInfo,flagDoContinuum,flagDoHI,minMinorAxis)

    f = file(imageParsetFile,"w")
    f.write(createFITSinput)
    f.close()

    if(makeImage):
        print "About to run createFITS to make the FITS file."
        pathToSims = "%s/Code/Components/Analysis/simulations/trunk/install/bin"%os.environ['ASKAP_ROOT']
        createFITScall = "%s/createFITS.sh -inputs %s"%(pathToSims,imageParsetFile)
        os.system(createFITScall)
