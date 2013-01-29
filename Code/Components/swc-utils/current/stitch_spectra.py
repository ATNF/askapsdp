# helper script to run delaytest.sh for a number of CONTROL selection settings and stitch 
#    the resulting averaged spectrum into a single file (with reordering to account for inversion)

prog = "apps/delaytest.sh"

import os,sys

if len(sys.argv)!=2:
   raise RuntimeError, "Usage: %s measurement_set" % (sys.argv[0],)

msName = sys.argv[1]

if not os.path.exists(msName):
   raise RuntimeError, "Measurement set %s doesn't exist" % (msName,)

def runProg(central_freq, msName):
   """
      helper method to run delaytest.sh, parse the output and returns False if no data were found
       
      central_freq - central frequency in MHz to be used as CONTROL selection setting
      msName - measurement set

      Return: True, if some data were processed and False otherwise
   """
   tmpFile = ".tmp.delaytest"
   os.system("%s %i %s > %s" % (prog, central_freq, msName, tmpFile))
   noData = False
   f = open(tmpFile)
   try:
      for line in f:
         if "No data found!" in line:
            noData = True
   finally:
      f.close()
   return not noData


def loadAvgSpectrum():
   """
      helper method to load averaged spectrum (avgspectrum.dat) and return it as a dictionary with 
      key being the frequency and the value being the rest of the numbers as a tuple of strings

      Return: dictionary with the spectrum (use dictionary as the frequencies may come at random order)
   """
   result = {}
   spcFile = "avgspectrum.dat"
   if not os.path.exists(spcFile):
      raise RuntimeError, "File %s doesn't exist, perhaps runProg didn't work properly" % (spcFile,)
   f = open(spcFile)
   try:
     for line in f:
         parts = line.split()
         if len(parts)<3:
            raise RuntimeError, "Expect at least 3 columns in %s" % (spcFile,)
         freq = int(parts[1])
         if freq in result:
            raise RuntimeError, "Found duplicated frequency %i in the same output %s" % (freq, spcFile)
         result[freq] = tuple(parts[2:])   
   finally:
     f.close()
   return result

spectrum = {}
freqs = []
for central_freq in range(688, 1520, 16):
    if runProg(central_freq, msName):
       chunk = loadAvgSpectrum()
       for f in chunk:
           if f in freqs:
              print "WARNING! Found duplicated frequency %i MHz for central frequency %i MHz, last data will be used" % (f,central_freq)
           else:
              freqs.append(f)
       spectrum.update(chunk)
    else:
       print "No data for central frequency %i MHz" % (central_freq,)

# now sort frequencies and write the output in the ordered way
freqs.sort()
f = open("stitched_spectrum.dat","w")
try:
   for freq in freqs:
       f.write("%i %s\n" % (freq, ' '.join(spectrum[freq])))
finally:
   f.close()


