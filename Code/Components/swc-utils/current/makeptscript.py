import sys, os

if len(sys.argv)!=2:
   raise RuntimeError, "Usage: %s file" % (sys.argv[0],)

f = open(sys.argv[1])
try:
  print "#!/bin/sh"
  for line in f:
     if line.endswith("\n"):
           line = line[:-1]
     pos = line.find("applied to test direction:")
     if pos != -1:
        offsets=line[pos+27:]
        parts = offsets.split()
        if len(parts)!=2:
           raise RuntimeError, "Expect two numbers as offsets"
        pos1 = parts[1].find(".")
        if pos1 == -1:
           raise RuntimeError, "Expect dot-separated declination"
        pos2 = parts[1].find(".",pos1+1)
        if pos2 == -1:
           raise RuntimeError, "Expect dot-separated declination"
        decstr = parts[1][:pos1] + ":" + parts[1][pos1+1:pos2] + ":" + parts[1][pos2+1:]
        print ""
        print "echo Pointing at %s %s" % (parts[0], decstr)
        print "# %s" % line
        print "~/point.sh %s %s pa_fixed 0" % (parts[0], decstr)
        print "sleep 60"   
finally:
  f.close()

