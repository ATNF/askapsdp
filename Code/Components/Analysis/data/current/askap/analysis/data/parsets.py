#!/usr/bin/env python

import math
from numpy import *

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

