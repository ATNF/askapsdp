#!/usr/bin/env python

from pkg_resources import require
require('MySQL_python')
import MySQLdb
require('numpy')
from numpy import *
import os

S3SEX_host = "gijane"
S3SEX_user = "skads"
S3SEX_pass = "skads"
S3SEX_db = "S3SEX"

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
        
    fullresults = array([])
    for type in types:
        for x in centres:
            for y in centres:
                
                tabname = getTabName(x,y,type)

                query = "SELECT right_ascension,declination,pow(10,i_1400) as flux,major_axis,minor_axis,position_angle FROM %s WHERE i_1400>=-6."%(tabname)
                print query

                cursor.execute(query)
                results=array(cursor.fetchall())
                print shape(results)

                for r in results:  # This fixes sources where major_axis>0 yet minor_axis=0: infinite axial ratio!
                    if(r[3]>0 and r[4]==0.):
                        r[4] = 0.0005
                
                if(len(results)>0):
                    if(shape(fullresults)==(0,)): #if this is the first lot of results
                        fullresults = results
                    else:
                        fullresults = append(fullresults,results,0)

    print shape(fullresults)

    catalogueFilename = "data/SKADS_S3SEX_10sqdeg_1uJy.dat"
    print "Saving full lot of results to file: %s"%catalogueFilename
    savetxt(catalogueFilename, fullresults)


############
# Make FITS image
#   use simFITS code in askapsoft

    imageName = "data/SKADS_S3SEX_10sqdeg_1uJy.fits"
    createFITSinput = """
createFITS.filename         = %s
createFITS.sourcelist       = %s
createFITS.posType          = deg
createFITS.addNoise         = false
createFITS.bunit            = JY/PIXEL
createFITS.dim              = 4
createFITS.axes             = [8192,8192,1,1]
createFITS.ctype            = [RA---SIN, DEC--SIN, STOKES, FREQ-OBS]
createFITS.cunit            = [deg, deg, "", Hz]
createFITS.crval            = [0., 0., 1., 1.2453125e9]
createFITS.crpix            = [4095., 4095., 1., 1.]
createFITS.crota            = [0., 0., 0., 0.]
createFITS.cdelt            = [-0.00122, 0.00122, 1., 1.2453125e9]
createFITS.doConvolution    = false
""" % (imageName, catalogueFilename)

    f = file("createFITS.in","w")
    f.write(createFITSinput)
    f.close()

    print "About to run createFITS to make the FITS file."
    createFITScall = "%s/Code/Components/Analysis/simulations/simFITS/trunk/apps/createFITS.sh"%os.environ["ASKAP_ROOT"]
    os.system(createFITScall)
