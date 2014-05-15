# rotate antenna positions (to get coordinates used by the delay script

import os, sys, math

if len(sys.argv)<2:
   raise RuntimeError, "Usage %s parset.in" % sys.argv[0]

lng=116.631786 / 180. * math.pi

f = file(sys.argv[1])
try:
   cl = math.cos(lng)
   sl = math.sin(lng)
   for line in f:
      if line.startswith("swcorrelator.antennas"):
         if line.find("names")!=-1:
             continue
         pos1 = line.find("[")
         pos2 = line.find("]")
         pos0 = line.find("=")
         if pos1 == -1 or pos2 == -1 or pos0 == -1:
            continue
         arr = line[pos1+1:pos2]
         parts = arr.split(",")
         if len(parts) != 3:
            raise RuntimeError, "Expecting 3 numbers,  you have %s" % (parts,)
         Lx = float(parts[0])
         Ly = float(parts[1])
         Lz = float(parts[2])
         Lx1 = Lx*cl + Ly*sl
         Lx2 = -Lx*sl + Ly*cl
         print "%s : (%f,%f,%f)"%(line[:pos0],Lx1,Lx2,Lz)
finally:
   f.close()
