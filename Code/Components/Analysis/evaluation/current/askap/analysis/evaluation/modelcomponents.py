#!/usr/bin/env python

## @file
#  A file containing definitions of model components

from math import *
from utils import *

## @namespace modelcomponents
#  Model components used by the analysis & cross-matching code, that
#  will be referenced by the evaluation plots.

class SelavyObject:
    def __init__(self,line):
        self.line = line
        if(line[0]!='#'):
            cols = line.split()
            self.id = cols[0]
            self.name = cols[1]
            self.ra = float(cols[2])
            self.dec = float(cols[3])
            self.x = float(cols[4])
            self.y = float(cols[5])
            self.Fint = float(cols[6])
            self.Fpeak = float(cols[7])
            self.FintFIT = float(cols[8])
            self.FpeakFIT = float(cols[9])
            self.maj = float(cols[10])
            self.min = float(cols[11])
            self.pa = float(cols[12])
            self.majDECONV = float(cols[13])
            self.minDECONV = float(cols[14])
            self.paDECONV = float(cols[15])
            self.alpha = float(cols[16])
            self.beta = float(cols[17])
            self.chisqFIT = float(cols[18])
            self.RMSimage = float(cols[19])
            self.rmsFIT = float(cols[20])
            self.nfreeFIT = int(cols[21])
            self.ndofFIT = int(cols[22])
            self.npixFIT = int(cols[23])
            self.npixObj = int(cols[24])
            self.guess = int(cols[25])

    def flux(self):
        return self.FintFIT

            

class ContinuumObject:
    def __init__(self,line):
        self.line = line
        if(line[0]!='#'):
            cols=line.split()
            self.ra = posToDec(cols[0])
            self.dec = posToDec(cols[1])
            self.flux0 = float(cols[2])
            self.alpha = float(cols[3])
            self.beta = float(cols[4])
            self.maj = float(cols[5])
            self.min = float(cols[6])
            self.pa = float(cols[7])
            self.id='%s_%s'%(cols[0],cols[1])
            
    def flux(self):
        return self.flux0

class ContinuumIDObject:
    def __init__(self,line):
        self.line = line
        if(line[0]!='#'):
            cols=line.split()
            self.id = cols[0]
            self.ra = posToDec(cols[1])
            self.dec = posToDec(cols[2])
            self.flux0 = float(cols[3])
            self.alpha = float(cols[4])
            self.beta = float(cols[5])
            self.maj = float(cols[6])
            self.min = float(cols[7])
            self.pa = float(cols[8])
            
    def flux(self):
        return self.flux0

    
class Match:
    def __init__(self,line,srcCat,refCat):
        cols=line.split()
        self.src=srcCat[cols[1]]
        self.ref=refCat[cols[2]]
        self.sep=float(cols[3])
        self.type=int(cols[0])
        self.setOffsets()

    def setOffsets(self,flagSphericalPos=True):
        self.dx=(self.src.ra-self.ref.ra)
        self.dy=(self.src.dec-self.ref.dec)
        if(flagSphericalPos):
            self.dx = self.dx * cos(0.5*(self.src.dec+self.ref.dec)*pi/180.)
