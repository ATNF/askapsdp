# regression tests with gridders taking w-term into account
# some fixed parameters are given in wtermtest_template.in

import os,math

# helper class to run a program from synthesis
# can be moved into a separate file eventually
class SynthesisProgramRunner:
   
   def __init__(self, template_parset = 'wtermtest_template.in'):
      '''
         initialise the class
 
         template_parset - file name for the template parset file
                  containing all parameters, which are not supposed to
                  change 
      '''
      if not os.path.exists(template_parset):
         raise RuntimeError, "Template parset file %s is not found" % template_parset
      self.template_parset = template_parset

      if 'ASKAP_ROOT' not in os.environ:
          raise RuntimeError, "ASKAP_ROOT should be initialised first!"

      if 'AIPSPATH' not in os.environ:
         os.environ['AIPSPATH'] = os.path.join(os.environ['ASKAP_ROOT'],'Code/Components/Synthesis/testdata/trunk')
      self.simulator = os.path.join(os.environ['ASKAP_ROOT'],'Code/Components/Synthesis/synthesis/trunk/install/bin/csimulator.sh')
      self.imager = os.path.join(os.environ['ASKAP_ROOT'],'Code/Components/Synthesis/synthesis/trunk/install/bin/cimager.sh')
      self.imgstat = os.path.join(os.environ['ASKAP_ROOT'],'Code/Components/Synthesis/synthesis/trunk/install/bin/imgstat.sh')

      if not os.path.exists(self.simulator):
          raise RuntimeError, "csimulator is missing at %s" % self.simulator

      if not os.path.exists(self.imager):
          raise RuntimeError, "cimager is missing at %s" % self.imager

      if not os.path.exists(self.imgstat):
          raise RuntimeError, "imgstat is missing at %s" % self.imgstat

      self.tmp_parset = "temp_parset.in"
      self.initParset()
      
   
   def initParset(self):
      '''
         Initialise temporary parset to the template

      '''
      if os.path.exists(self.tmp_parset):
         print "WARNING. File %s is overwritten" % self.tmp_parset
      os.system("rm -f %s" %  self.tmp_parset)
      os.system("cp %s %s" % (self.template_parset, self.tmp_parset))
      

   def addToParset(self,str):
      '''
         Add the given string to the temporary parset file (created in
         the constructor and passed to all commands executed throughout
         the lifetime of this object
  
         str string to add
      '''
      os.system("echo \'%s\' >> %s" % (str, self.tmp_parset))

   def runCommand(self,cmd):
      '''
         Run given command on a current parset

         cmd - command
      '''
      res = os.system("%s -inputs %s" % (cmd, self.tmp_parset))
      if res != 0:
         raise RuntimeError, "Command %s failed with error %s" % (cmd,res)

   def runSimulator(self):
      '''
         Run csimulator on a current parset
      '''
      self.runCommand(self.simulator)
         

   def runImager(self):
      '''
         Run cimager on a current parset
      '''
      self.runCommand(self.imager)

   def imageStats(self, name):
      '''
         Get image statistics

         name - image name
      '''
      if not os.path.exists(name):
         raise RuntimeError, "Image %s doesn't exist" % name
      imgstat_out = ".tmp.imgstat"
      if os.path.exists(imgstat_out):
         os.system("rm -f %s" % imgstat_out)
      res = os.system("%s %s > %s" % (self.imgstat,name,imgstat_out))
      if res != 0:
         raise RuntimeError, "Command %s failed with error %s" % (self.imgstat,res)
      result = {}
      f = file(imgstat_out)
      try:
         row = 0
         for line in f:
            parts = line.split()
            if len(parts)<2 and row>0:
                   raise RuntimeError, "Expected at least 2 elements in row %i, you have: %s" % (row+1,parts)
            if row == 0:
               if len(parts)<4:
                  raise RuntimeError, "Expected at least 4 columns on the first row, you have: %s " % (parts,)
               result['peak'] = float(parts[0])
               if parts[3] != "(J2000)":
                   raise RuntimeError, "Expected J2000 as the 4th element, you have: %s " % (parts,) 
            elif row == 1:
               result['ra'] = float(parts[0])
               result['dec'] = float(parts[1])
            elif row == 2:
               result['rms'] = float(parts[0])
               result['median'] = float(parts[1])
            row = row + 1
      finally:
         f.close()
      return result
      
# angular distance in degrees of the peak from the given point
# ra and dec are J2000 coordinates in degrees
# stats - dictionary with 'ra' and 'dec' field giving the position of the peak
def getDistance(stats, ra, dec):
   ra1 = stats['ra']
   dec1 = stats['dec']
   cosd = math.sin(math.pi/180.*dec1)*math.sin(math.pi/180.*dec)+math.cos(math.pi/180.*dec1)*math.cos(math.pi/180.*dec)* math.cos(math.pi/180.*(ra1-ra));
   return math.acos(cosd)*180./math.pi;

# formulae for SIN-projection to test that position is at the right spot
def sinProjection(ref,l,m):
   '''
      ref - a list or tuple with 2 elements being ra and dec in degrees of the tangent point
      l,m - offsets in the tangent plane in degrees

      Return: two element tuple with ra and dec of the offset position (degrees)
   '''
   if len(ref)!=2:
      raise RuntimeError, "Expected two-element list or tuple, you have: %s" % (ref,)
   # offsets in radians
   L = l/180.*math.pi
   M = m/180.*math.pi
   # sin and cos of ref. declination
   cDelta0 = math.cos(ref[1]/180.*math.pi)
   sDelta0 = math.sin(ref[1]/180.*math.pi)
   # actual result
   dec = math.asin(M*cDelta0+sDelta0*math.sqrt(1.-L*L-M*M))/math.pi*180.
   ra = ref[0] + math.atan2(L,cDelta0*math.sqrt(1.-L*L-M*M)-M*sDelta0)/math.pi*180.
   return (ra,dec)

def analyseResult(spr):
   '''
      spr - synthesis program runner (to run imageStats)

      throws exceptions if something is wrong, otherwise just
      returns
   '''
   src_offset = 0.006/math.pi*180.
   psf_peak=[-172.5,-45]
   true_peak=sinProjection(psf_peak,src_offset,src_offset)
   stats = spr.imageStats('image.field1.restored')
   print "Statistics for restored image: ",stats
   disterr = getDistance(stats,true_peak[0],true_peak[1])*3600.
   if disterr > 8:
      raise RuntimeError, "Offset between true and expected position exceeds 1 cell size (8 arcsec), d=%f, true_peak=%s" % (disterr,true_peak)
   if abs(stats['peak']-1.)>0.01:
      raise RuntimeError, "Peak flux in the image is notably different from 1 Jy, F=%f" % stats['peak']

   stats = spr.imageStats('image.field1')
   print "Statistics for modelimage: ",stats
   disterr = getDistance(stats,true_peak[0],true_peak[1])*3600.
   if disterr > 8:
      raise RuntimeError, "Offset between true and expected position exceeds 1 cell size (8 arcsec), d=%f, true_peak=%s" % (disterr,true_peak)

   stats = spr.imageStats('psf.field1')
   print "Statistics for psf image: ",stats
   disterr = getDistance(stats,psf_peak[0],psf_peak[1])*3600.
   if disterr > 8:
      raise RuntimeError, "Offset between true and expected position exceeds 1 cell size (8 arcsec), d=%f, true_peak=%s" % (disterr,true_peak)

   stats = spr.imageStats('psf.image.field1')
   print "Statistics for preconditioned psf image: ",stats
   disterr = getDistance(stats,psf_peak[0],psf_peak[1])*3600.
   if disterr > 8:
      raise RuntimeError, "Offset between true and expected position exceeds 1 cell size (8 arcsec), d=%f, true_peak=%s" % (disterr,true_peak)
   if abs(stats['peak']-1.)>0.01:
      raise RuntimeError, "Peak flux in the preconditioned psf image is notably different from 1.0, F=%f" % stats['peak']

   stats = spr.imageStats('weights.field1')
   print "Statistics for weight image: ",stats
   if abs(stats['rms']-stats['peak'])>0.1 or abs(stats['rms']-stats['median'])>0.1 or abs(stats['peak']-stats['median'])>0.1:
      raise RuntimeError, "Weight image is expected to be constant for WProject and WStack gridders"


spr = SynthesisProgramRunner()
spr.runSimulator()

spr.addToParset("Cimager.gridder = WProject")
spr.runImager()
analyseResult(spr)

spr.initParset()
spr.addToParset("Cimager.gridder = WStack")
spr.runImager()
analyseResult(spr)
