#!/usr/bin/env python

import askap.analysis.data
from numpy import *
import math
from optparse import OptionParser
import os
import askap.parset as parset

def sphericalDistance(ra1, dec1, ra2, dec2):
      r1 = ra1  * math.pi / 180.;
      d1 = dec1 * math.pi / 180.;
      r2 = ra2  * math.pi / 180.;
      d2 = dec2 * math.pi / 180.;
      angsep = math.cos(r1-r2)*math.cos(d1)*math.cos(d2) + math.sin(d1)*math.sin(d2);
      return math.acos(angsep)*180./math.pi;


def writeData(line, fileno, subfile, doAnn, annfile, threshold, count=0):

    subfile[fileno].write("%s"%line)
    data=array(line.split()).astype(float)
    if(doAnn):
        annfile[fileno].write("#Object details: %s"%line)
        if(data[3]>0.):
#            annfile[fileno].write("ELLIPSE %12.8f %12.8f %10.8f %10.8f %10.8f\nTEXT %12.8f %12.8f %d\n"%(data[0],data[1],data[3]/3600.,data[4]/3600.,data[5]*math.pi/180.,data[0],data[1],count))
            annfile[fileno].write("ELLIPSE %12.8f %12.8f %10.8f %10.8f %10.8f\nTEXT %12.8f %12.8f %d\n"%(data[0],data[1],data[3]/2./3600.,data[4]/2./3600.,data[5]*180./math.pi,data[0],data[1],count))
        else:
            annfile[fileno].write("CIRCLE %12.8f %12.8f %10.8f\nTEXT %12.8f %12.8f %d\n"%(data[0],data[1],10./3600.,data[0],data[1],count))

############

if __name__ == '__main__':

    parser = OptionParser()
    parser.add_option("-i","--inputs", dest="inputfile", default="createSubLists.in", help="Input parameter file [default: %default]")

    (options, args) = parser.parse_args()

    if(not os.path.exists(options.inputfile)):
        print "Input file %s does not exist!"%options.inputfile
        exit(1)

    inparams = parset.ParameterSet(options.inputfile).createSubs

    if('catfilename' not in inparams):
        print "%s has not specified the catalogue filename via the catfilename parameter.\nExiting\n"%options.inputfile
        exit(1)

    doAnnFile = inparams.get_value('flagAnnotation',True)
    doPtsOnly = inparams.get_value('flagPointSources',False)
    thresh = inparams.get_value('thresholds',[0.])
    print "thresholds=",thresh
    radii = inparams.get_value('radii',[])
    print "radii=",radii
    catfilename = inparams.get_value('catfilename')
    racentre = inparams.get_value('racentre',187.5)
    deccentre = inparams.get_value('deccentre',-45.)
    destDir = inparams.get_value('destDir','.')
    
    catfile = file(catfilename, 'rU')
    
    baseCatName = catfilename.split('/')[-1]
    if(baseCatName.rfind('.')<0):
        baseOutFile = destDir+'/'+baseCatName
    else:
        baseOutFile = destDir+'/'+baseCatName[:baseCatName.rfind('.')]

    if(catfilename == ''):
        print "No input catalogue given. Exiting."
        exit(0)

    subfilenames =  []
    annfilenames =  []
    if(len(thresh)==0):
          print "No thresholds given. Exiting."
          exit(0)

    for thr in thresh:
        suffix=''
        if(doPtsOnly):
            suffix='_pt'
        if(thr>0.):
            suffix += '_%duJy'%thr
        if(thr>0.):
              subfilenames.append(baseOutFile + '%s.txt'%suffix)
              if(doAnnFile):
                    annfilenames.append(baseOutFile + '%s.ann'%suffix)
        if(len(radii)>0):
            for r in radii:
                subfilenames.append(baseOutFile + '%s_%3.1fdeg.txt'%(suffix,r))
                if(doAnnFile):
                    annfilenames.append(baseOutFile + '%s_%3.1fdeg.ann'%(suffix,r))
    
    subfiles = []
    print "Creating the following files:"
    for name in subfilenames:
          print name
          subfiles.append(file(name,'w'))
    annfiles = []
    for name in annfilenames:
          print name
          annfiles.append(file(name,'w'))
          annfiles[-1].write("COORD w\nPA SKY\nCOLOR SEA GREEN\nFONT lucidasans-10\n")
        
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
                      if(thr>0):
                            if(float(data[2])*1.e6>=thr):
                                  writeData(line,filecount,subfiles,doAnnFile,annfiles,thr,sourcecount)
                            filecount += 1
                      if(len(radii)>0):
                        for r in radii:
                            if( float(data[2])*1.e6>=thr and dist<r ):
                                writeData(line,filecount,subfiles,doAnnFile,annfiles,thr,sourcecount)
                            filecount += 1

    for file in subfiles:
        file.close()
    for file in annfiles:
        file.close()
