#!/usr/bin/env python
import sys
from askapdev.epicsdb import AdbeParser

parser = AdbeParser('$(prefix)$(antid)$(ss)')
parser.parse(sys.argv[1])
parser.tree.write('resolved.xml')

parser.generate_output(sys.argv[2])
print(parser.paramList)

#with open('test.h', 'w') as f:
#    parser.epicsDb.save_to_asyn_header(f)

parser.dump_pvs()

numPVs = len(parser.epicsDb)
print("Num PVs      : ", numPVs)
print("Max PV Name  : ", parser.maxLength)
print("PV types     : ", parser.recordTypes.keys())
