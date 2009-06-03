#!/usr/bin/env python

from pkg_resources import require
require('numpy')
from numpy import *
import math
from optparse import OptionParser

def getInputParams(infilename, prefix):
    paramlist = {}
    infile = file(infilename,'rU')
    lines = infile.readlines()
    for line in lines:
        if(line[:line.find('.')+1]==prefix):
            key = line.split('=')[0][line.find('.')+1:]
            val = line.split('=')[1]
            paramlist[key.strip()] = val.strip()
    return paramlist

def getParamValue(dict, param, default):
    if(not dict.has_key(param)):
        return default
    val = dict[param]
    return val        

def getParamArray(dict, param, default):
    if(not dict.has_key(param)):
        return default
    val = dict[param]
    if(val[0]!='['):
        return default
    if(val[-1]!=']'):
        return default
    return array(val.replace('[','').replace(']','').split(','))


def sphericalDistance(ra1, dec1, ra2, dec2):
      r1 = ra1  * math.pi / 180.;
      d1 = dec1 * math.pi / 180.;
      r2 = ra2  * math.pi / 180.;
      d2 = dec2 * math.pi / 180.;
      angsep = math.cos(r1-r2)*math.cos(d1)*math.cos(d2) + math.sin(d1)*math.sin(d2);
      return math.acos(angsep)*180./math.pi;


def writeData(line, subfile, doAnn, annfile, threshold, radius=0., count=0):

    subfile.write("%s\n"%line)
    data=array(line.split()).astype(float)
    if(doAnn):
        if(data[3]>0.):
            annfile.write("ELLIPSE %12.8f %12.8f %10.8f %10.8f %10.8f\nTEXT %12.8f %12.8f %d\n"%(data[0],data[1],data[3],data[4],data[5],data[0],data[1],count))
        else:
            annfile.write("CIRCLE %12.8f %12.8f %10.8f\nTEXT %12.8f %12.8f %d\n"%(data[0],data[1],10./3600.,data[0],data[1],count))

############

if __name__ == '__main__':

    parser = OptionParser()
    parser.add_option("-i","--inputs", dest="inputfile", default="createSubLists.in")

    (options, args) = parser.parse_args()

    inparams = getInputParams(options.inputfile, "createSubs.")

    nullarray = array([0.])
    doAnnFile = (getParamValue(inparams, 'flagAnnotation', 'true').lower() == 'true')
    doPtsOnly = (getParamValue(inparams, 'flagPointSources', 'false').lower() == 'true')
    thresh = getParamArray(inparams, 'thresholds', nullarray).astype(float)
    radii = getParamArray(inparams, 'radii', nullarray).astype(float)
    catfilename = getParamValue(inparams, 'catfilename', '')
    racentre = float(getParamValue(inparams, 'racentre', 187.5))
    deccentre = float(getParamValue(inparams, 'deccentre', -45.))
    destDir = getParamValue(inparams, 'destDir', '.')
    
    catfile = file(catfilename, 'rU')
    
    baseCatName = catfilename.split('/')[-1]
    baseOutFile = destDir+'/'+baseCatName[:baseCatName.rfind('.')]

    if(catfilename == ''):
        print "No input catalogue given. Exiting."
        exit(0)

    subfilenames =  []
    annfilenames =  []
    for thr in thresh:
        suffix=''
        if(doPtsOnly):
            suffix='_pt'
        suffix += '_%duJy'%thr
        subfilenames.append(baseOutFile + '%s.txt'%suffix)
        annfilenames.append(baseOutFile + '%s.ann'%suffix)
        for r in radii:
            subfilenames.append(baseOutFile + '%s_%ddeg.txt'%(suffix,r))
            annfilenames.append(baseOutFile + '%s_%ddeg.ann'%(suffix,r))
    
    subfiles = []
    for name in subfilenames:
        subfiles.append(file(name,'w'))
    annfiles = []
    for name in annfilenames:
        annfiles.append(file(name,'w'))
        annfiles[-1].write("COORD w\nCOLOR SEA GREEN\nFONT lucidasans-10\n")
        
    datalines = catfile.readlines()
    sourcecount = 0;
    for line in datalines:
        if(line[0]!='#'):
            data = line.split()
            if((not doPtsOnly) or float(data[3])==0):
                dist = sphericalDistance(float(data[0]),float(data[1]),racentre,deccentre)
                filecount=0;
                sourcecount += 1;
                for thr in thresh:
                    if(float(data[2])*1.e6>=thr):
                        writeData(line,subfiles[filecount],doAnnFile,annfiles[filecount],thr)
                    filecount += 1
                    for r in radii:
                        if( float(data[2])*1.e6>=thr and dist<r ):
                            writeData(line,subfiles[filecount],doAnnFile,annfiles[filecount],thr,r,sourcecount)
                        filecount += 1

    for file in subfiles:
        file.close()
    for file in annfiles:
        file.close()
