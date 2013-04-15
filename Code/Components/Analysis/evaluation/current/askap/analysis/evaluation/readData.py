#!/usr/bin/env python

## @file
#  A file containing functions to read in data ready for plotting. It
#  can read data for matching sources and sources that didn't match.

## @namespace readData
#  Data I/O for python scripts.
#  A set of functions to read in the results of the pattern matching,
#  so that we can produce nice plots showing how well the source list
#  matches the reference list.

from pkg_resources import require
require('numpy')
require('matplotlib')
from pylab import *
from numpy import *
from askap.analysis.evaluation.modelcomponents import *
import logging

#############################################################################

## @ingroup plotting
# Reads the set of matches, returning three dictionaries: the source
# and reference dictionaries (ID matched to the appropriate object),
# plus a dictionary matching IDs of matched objects
def readMatches(matchfile,srcDict,refDict):

    fin=open(matchfile)
    matchlist=[]
    for line in fin:
        cols=line.split()
        matchlist.append(Match(line,srcDict,refDict))

    return matchlist

def readMisses(missfile,catalogue,key):
    fin=open(missfile)
    misslist=[]
    for line in fin:
        cols=line.split()
        if(cols[0]==key):
            misslist.append(catalogue[cols[1]])

    return misslist


## @ingroup plotting
# Reads a catalogue of objects
def readCat(filename,catalogueType):

    if(catalogueType == "Selavy" or catalogueType=="Continuum" or catalogueType=="ContinuumID"):

        catDict={}
        fin=open(filename)
        for line in fin:
            if(line[0]!='#'):
                if(catalogueType=="Selavy"):
                    obj=SelavyObject(line)
                elif(catalogueType=="Continuum"):
                    obj=ContinuumObject(line)
                elif(catalogueType=="ContinuumID"):
                    obj=ContinuumIDObject(line)
                catDict[obj.id] = obj
        fin.close()

    else:
        logging.error("Catalogue type %s not known"%catalogueType)

    return catDict

