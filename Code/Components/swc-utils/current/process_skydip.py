# quick and dirty script to extract some initial data for the skydip measurement
# it runs delaytest.sh for a number of "CONTROL" selection settings (assumed translated
# to scan numbers), extracts data for a given channel and writes the result vs. elevation
# It is assumed that delaytest is compiled the right way (autocorrelations selected, given beam chosen, etc)

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
         freq = float(parts[1])
         if abs(freq - int(freq))>0.01:
            raise RuntimeError, "Expect frequency to be integral number of MHz, you have %f" % (freq,)
         freq = int(freq)
         if freq in result:
            raise RuntimeError, "Found duplicated frequency %i in the same output %s" % (freq, spcFile)
         result[freq] = tuple(parts[2:])   
   finally:
     f.close()
   return result

def extractAmplitude(chan = 250): 
   """
      Extract amplitude and its uncertainty for a given channel one pair per antenna
   """
   spc = loadAvgSpectrum()
   if len(spc) <= chan:
      raise RuntimeError, "loaded only %i channels" % (len(spc),)
   keys = [v for v in spc]
   print " channel %i corresponds to %f MHz" % (chan, keys[chan])
   res = spc[keys[chan]]
   if len(res) % 3 != 0:
      raise RuntimeError, "Expect 3 elements for each antenna in the avgspectrum, you have %s" % (res,)
   amps = []
   for ant in range(len(res)/3):
       amps.append(float(res[ant*3]))
       amps.append(float(res[ant*3+2]))
   return amps
   
with open("skydip.dat","w") as f:
  for scan in range(1,30):
     posID = scan
     if scan > 15:
        posID = 30 - scan   
     if posID < 1:
        raise RuntimeError, "This is not expected to happen"
     if posID % 2 == 0:
        elev = 23. + 5. * (posID - 2)
     else:
        elev = 17. + 5. * (posID - 1)
     print "Processing scan %i, elevation %f deg" % (scan, elev)

     if runProg(scan, msName):
        amps = extractAmplitude()
        if len(amps) % 2 != 0:
           raise RuntimeError, "Expect two elements per antenna, you have %s" % (amps,)
        f.write("%f %s\n" % (elev, ' '.join([str(a) for a in amps])))
     
     else:
        print "WARNING! No data for scan %i MHz" % (scan,)

