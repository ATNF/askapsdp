#!/usr/bin/env python

from pkg_resources import require
require('MySQL_python')
import MySQLdb
require('numpy')
from numpy import *
import os

S3SEX_host = "gijane.atnf.csiro.au"
S3SEX_user = "skads"
S3SEX_pass = "skads"
S3SEX_db = "S3SEX"

SuperBrightSource = 67005861  #component number of the source with 1400MHz flux = 584Jy

def getTabName(x,y,type):
    
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


    db = MySQLdb.connect(host=S3SEX_host, user=S3SEX_user, passwd=S3SEX_pass, db=S3SEX_db)
    cursor = db.cursor()
    
    types = ['RQAGN','FRI','FRII','SBG','SFG']
    centres = range(-5,6,1)
        
    catalogueFilenameOrig = "catalogues/SKADS_S3SEX_10sqdeg_1uJy_orig.dat"  # This is the name of the file that has data from the database, with positions centred on (0,0)
    catalogueFilename = "catalogues/SKADS_S3SEX_10sqdeg_1uJy.dat"           # This is the name of the file createFITS will create with corrected positions
    catfile = file(catalogueFilenameOrig,"w")
    catfile.write("#%9s %10s %20s %10s %10s %10s\n"%("RA","Dec","Flux","Maj_axis","Min_axis","Pos_ang"))
    for type in types:
        for x in centres:
            for y in centres:
                
                tabname = getTabName(x,y,type)

                query = "SELECT right_ascension,declination,pow(10,i_1400) as flux,major_axis,minor_axis,position_angle,component FROM %s WHERE i_1400>=-6."%(tabname)
                print query

                cursor.execute(query)
                results=array(cursor.fetchall())
                print shape(results)

                for r in results: 
                    if(r[3]>0 and r[4]==0.):  
                        # This fixes sources where major_axis>0 yet minor_axis=0: infinite axial ratio!
                        r[4] = 0.0005

                    if(r[6] == SuperBrightSource): 
                        # This fixes the super-bright source, on the assumption that its fluxes are lacking a minus sign
                        r[2] = 1. / r[2]

                    catfile.write("%10.6f %10.6f %20.16f %10.6f %10.6f %10.6f\n"%(r[0],r[1],r[2],r[3],r[4],r[5]))

    catfile.close()

############
# Make FITS image
#   use simFITS code in askapsoft

    imageName = "images/SKADS_S3SEX_10sqdeg_1uJy.fits"
    newRAcentre = 187.5
    newDECcentre = -45.
    createFITSinput = """\
createFITS.filename         = !%s
createFITS.sourcelist       = %s
createFITS.posType          = deg
createFITS.bunit            = JY/PIXEL
createFITS.dim              = 4
createFITS.axes             = [8192,8192,1,1]
createFITS.WCSimage.ctype   = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
createFITS.WCSimage.cunit   = [deg, deg, "", Hz]
createFITS.WCSimage.crval   = [%5.1f, %5.1f, 1., 1.2453125e9]
createFITS.WCSimage.crpix   = [4095., 4095., 1., 1.]
createFITS.WCSimage.crota   = [0., 0., 0., 0.]
createFITS.WCSimage.cdelt   = [-0.00122, 0.00122, 1., 1.2453125e9]
createFITS.outputList       = true
createFITS.outputSourceList = %s
createFITS.WCSsources       = true
createFITS.WCSsources.ctype = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
createFITS.WCSsources.cunit = [deg, deg, "", Hz]
createFITS.WCSsources.crval = [0., 0., 1., 1.2453125e9]
createFITS.WCSsources.crpix = [4095., 4095., 1., 1.]
createFITS.WCSsources.crota = [0., 0., 0., 0.]
createFITS.WCSsources.cdelt = [-0.00122, 0.00122, 1., 1.2453125e9]
createFITS.addNoise         = false
createFITS.doConvolution    = false
""" % (imageName, catalogueFilenameOrig, newRAcentre, newDECcentre, catalogueFilename)

    f = file("parsets/createFITS_SKADS.in","w")
    f.write(createFITSinput)
    f.close()

    print "About to run createFITS to make the FITS file."
    createFITScall = "%s/Code/Components/Analysis/simulations/trunk/apps/createFITS.sh -inputs parsets/createFITS_SKADS.in"%os.environ["ASKAP_ROOT"]
    os.system(createFITScall)
