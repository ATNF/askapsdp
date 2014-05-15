#!/usr/bin/env python
#
# Epics Database creation from CSV front end
#
# @copyright (c) 2011 CSIRO
# Australia Telescope National Facility (ATNF)
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# PO Box 76, Epping NSW 1710, Australia
# atnf-enquiries@csiro.au
#
# This file is part of the ASKAP software distribution.
#
# The ASKAP software distribution is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# @author Craig Haskins <Craig.Haskins@csiro.au>
#
'''
Command line front end for generating EPICS database
from CSV file according to format defined in 
http://pm.atnf.csiro.au/askap/projects/cmpt/wiki/DGS2EPICS_register_file
'''

import getopt, sys, os
from askapdev.epicsdb import EpicsDB

def usage():
    print
    print '%s --prefix="pvPrefix" --input="dgs.csv" --output="epics.db"' % os.path.basename(sys.argv[0])
    print
    print '     --prefix     - PV name prefix to apply to all PV names'
    print '     --input      - input csv file (can specify multiple times)'
    print '     --patch      - patch csv file for existing records (can specify multiple times)'
    print '     --output     - output EPICS db file to save to'
    print '     --no-comment - supress comments is output db'
            
if __name__ == '__main__':

    try:
        opts, args = getopt.getopt(sys.argv[1:],
            'dp:i:o:a:h:n',
            ['debug', 'prefix=', 'input=', 'output=', 'patch=', 'header=', 'header2=', 'seq=', 'no-comment'])
    except getopt.GetoptError:
        usage()
        sys.exit(2)

    outputType = None
    headerFile = None
    seqHeaderFile = None
    inputFiles = []
    prefix = ""
    debug = False
    supressComments = False
    external = False
    for opt, arg in opts:
        if opt in ('--input', '-i'):
            inputFiles.append(('input', arg))
        elif opt in ('--patch', '-a'):
            inputFiles.append(('patch', arg))
        elif opt in ('--output', '-o'):
            if '-' == arg:
                outputType = 'db'
                outputFile = sys.stdout
            else:
                outputType = os.path.splitext(arg)[1]
                outputFile = open(arg, 'w')
        elif opt in ('--header', '-h'):
                headerFile = arg
        elif opt in ('--header2'):
                headerFile = arg
                external = True
        elif opt in ('--seq', '-h'):
                seqHeaderFile = arg
        elif opt in ('--prefix', '-p'):
            prefix = arg
        elif opt in ('--debug', '-d'):
            debug = True
        elif opt in ('--no-comment', '-n'):
            supressComments = True
        else:
            print >> sys.stderr, 'unknown option', opt
            usage()
            sys.exit(0)

    if 0 == len(inputFiles) or not (outputType or headerFile):
        usage()
        sys.exit(0)

    db = EpicsDB(debug)
    
    # read in one or more input files
    for (inputType, fileName) in inputFiles:
        fileType = os.path.splitext(fileName)[1]
        update = ('patch' == inputType)
        with open(fileName, 'rU') as inputFile:
            if '.csv' == fileType:
                db.load_from_csv(inputFile, update)
            else:
                db.load_from_db(inputFile)

    # post process DB
    db.add_forward_links(chain=True)
    db.fixup_binary_records(chain=True)

    if None != seqHeaderFile:
        with open(seqHeaderFile, 'w') as f:
            db.save_to_seq_header(f)

    db.add_prefix(prefix)

    if '.csv' == outputType:
        db.save_to_csv(outputFile)
    elif '.db' == outputType:
        db.save_to_db(outputFile, supressComments)

    if None != headerFile:
        with open(headerFile, 'w') as f:
            db.save_to_asyn_header(f, externalDec=external)
