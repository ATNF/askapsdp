# runs simplecal.sh and aggregates results for the multibeam case
# this script is only necessary for the 3-antenna work and will not survive in the 
# long term - hence it is not very general (but committed to the tree, so it is not lost)

flux = 14.5
#basepath = "/Users/vor010/ASKAP/MS/Sep2013/subset"
basepath = "/Users/vor010/ASKAP/MS/Nov2013/subset"

import os

calutil = "/Users/vor010/ASKAP/ASKAPsoft/Code/Components/swc-utils/current/apps/simplecal.sh"

def extract(f, beam):
  with open('roughcalib.in') as tmpf:
     doAdd = False
     if beam == 0:
        doAdd = True
     for line in tmpf:
         if "Beam %i" % beam in line:
            doAdd = True
         if "Beam %i" % (beam+1,) in line:
            return
         if doAdd:
            f.write(line)

with open('roughmbcalib.in','w') as f:
   for beam in range(9):
      print "Processing beam %i" % (beam,)
      #calfile = os.path.join(basepath, "s1beam%ical.ms" % beam)
      calfile = os.path.join(basepath, "beam%ical.ms" % beam)
      os.system("%s %f %s" % (calutil, flux, calfile))
      f.write("# calibration file %s\n" % (calfile,))
      extract(f,beam)
