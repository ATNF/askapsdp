# regression tests with gridders taking w-term into account
# some fixed parameters are given in wtermtest_template.in

import os

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
      if os.path.exists(self.tmp_parset):
         print "WARNING. File %s is overwritten" % self.tmp_parset
      os.system("rm -f %s" %  self.tmp_parset)
      os.system("cp %s %s" % (self.template_parset, self.tmp_parset))

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
      

spr = SynthesisProgramRunner()
spr.runSimulator()
spr.runImager()
print spr.imageStats('image.field1.restored')
print spr.imageStats('image.field1')
