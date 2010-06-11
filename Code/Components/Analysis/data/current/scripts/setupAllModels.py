#!/usr/bin/env python

import os
import sys

if __name__ == '__main__':

    baseDir = '%s/Code/Components/Analysis/data/trunk'%os.environ['ASKAP_ROOT']
    scriptDir = baseDir+'/build/scripts-'+sys.version[:3]
    parsetDir = baseDir+'/parsets'

    createSKADS = scriptDir+'/createSKADS.py'
    sublists = scriptDir+'/createSubLists.py'
    createSKADSsub = sublists + ' --inputs ' + parsetDir + '/createSKADSsublist.in'
    create10uJysub = sublists + ' --inputs ' + parsetDir + '/createSubLists_10uJy_all.in'
    create10uJysubPts = sublists + ' --inputs ' + parsetDir + '/createSubLists_10uJy_pt.in'

    print "Creating the SKADS model catalogue + image ..."
    os.system(createSKADS)
    
    print "Creating the SKADS sublists ..."
    os.system(createSKADSsub)
    print "Creating the 10uJy model sublists with all sources ..."
    os.system(create10uJysub)
    print "Creating the 10uJy model sublists with just point sources ..."
    os.system(create10uJysubPts)
    

    
