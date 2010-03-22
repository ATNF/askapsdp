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
      if not os.path.exists(self.simulator):
          raise RuntimeError, "csimulator is missing at %s" % self.simulator

      if not os.path.exists(self.imager):
          raise RuntimeError, "cimager is missing at %s" % self.imager

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

spr = SynthesisProgramRunner()
spr.runSimulator()
spr.runImager()
