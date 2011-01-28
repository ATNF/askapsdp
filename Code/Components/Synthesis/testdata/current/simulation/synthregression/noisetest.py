# regression tests with mosaicing gridders doing single field
# imaging (essentially just primary beam correction, hence the name)
# some fixed parameters are given in pbcorrtest_template.in

from synthprogrunner import *
import math

def analyseResult(spr, expected_rms):
   '''
      spr - synthesis program runner (to run imageStats)

      throws exceptions if something is wrong, otherwise just
      returns
   '''
   stats = spr.imageStats('image.field1.restored')
   print "Statistics for restored image: ",stats
   if abs(stats['rms']-expected_rms)>0.02:
      raise RuntimeError, "rms in the image is notably different from %f Jy, rms=%f" % (expected_rms,stats['rms'])
   if abs(stats['median'])>expected_rms/10.:
      raise RuntimeError, "median exceeds expected_rms/10: expected_rms=%f Jy, median=%f" % (expected_rms,stats['median'])

   stats = spr.imageStats('residual.field1')
   print "Statistics for residual image: ",stats
   if abs(stats['rms']-expected_rms)>0.02:
      raise RuntimeError, "rms in the image is notably different from %f Jy, rms=%f" % (expected_rms,stats['rms'])
   if abs(stats['median'])>expected_rms/10.:
      raise RuntimeError, "median exceeds expected_rms/10: expected_rms=%f Jy, median=%f" % (expected_rms,stats['median'])



spr = SynthesisProgramRunner(template_parset = 'noisetest_template.in')
spr.runSimulator()

# we simulate 270 cycles (9 cuts, 5 min each, 10sec cycle) and 630 baselines
# and add gaussian noise with sigma=1e-3 Jy
ncycles = 270
nbaselines = 630
noisePerPol = 1e-3/math.sqrt(ncycles*nbaselines)
print "Noise per polarisation for %i cycles and %i baselines (sigma per vis is 1e-3 Jy) is %f Jy" % (ncycles, nbaselines,noisePerPol)

spr.addToParset("Cimager.Images.image.field1.polarisation = [\"XX\"]")
spr.runImager()
analyseResult(spr,noisePerPol)

# test noise figures after polarisation conversion
spr.initParset()
spr.addToParset("Cimager.Images.image.field1.polarisation = [\"I\"]")
spr.runImager()
analyseResult(spr,noisePerPol*math.sqrt(2.))

spr.initParset()
spr.addToParset("Cimager.Images.image.field1.polarisation = [\"Q\"]")
spr.runImager()
analyseResult(spr,noisePerPol*math.sqrt(2.))
