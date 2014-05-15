import os,sys,math

def convAngle(str):
   parts=str.split(':')
   if len(parts)!=3:
      raise RuntimeError, "unsupported coordinate string %s" % str
   if parts[0].find('-')!=-1:
      return float(parts[0])-float(parts[1])/60.-float(parts[2])/3600.
   else:
      return float(parts[0])+float(parts[1])/60.+float(parts[2])/3600.

if len(sys.argv) != 3:
   raise RuntimeError, "Usage: %s hour_angle dec" % sys.argv[0]

os.system("tail visplot.dat > .tmp.delaylog.dat")

f = open(".tmp.delaylog.dat")


# 3 - baselines
delay12 = 0.
delay23 = 0.
delay13 = 0.
sqdelay12 = 0.
sqdelay23 = 0.
sqdelay13 = 0.
count = 0
startTime = 0

for line in f:
  parts = line.split()
  if len(parts)>=10:
     if count == 0:
        startTime = float(parts[0])/1e6
     delay12 += float(parts[3])
     delay23 += float(parts[6])
     delay13 += float(parts[9])
     sqdelay12 += float(parts[3])**2
     sqdelay23 += float(parts[6])**2
     sqdelay13 += float(parts[9])**2
     count += 1

f = None

ha = convAngle(sys.argv[1])*15
dec = convAngle(sys.argv[2])

delay12 /= float(count)
delay23 /= float(count)
delay13 /= float(count)
sqdelay12 /= float(count)
sqdelay23 /= float(count)
sqdelay13 /= float(count)

uncert12 = 0
uncert23 = 0
uncert13 = 0

if count > 1:
   uncert12 = math.sqrt(sqdelay12 - delay12**2)
   uncert23 = math.sqrt(sqdelay23 - delay23**2)
   uncert13 = math.sqrt(sqdelay13 - delay13**2)


print "For HA=%f deg Dec=%f deg, averaged delays over %i cycles are:" % (ha,dec,count)
print "(1-2):      %f  +/- %f  ns" % (delay12, uncert12)      
print "(2-3):      %f  +/- %f  ns" % (delay23, uncert23)      
print "(1-3):      %f  +/- %f  ns" % (delay13, uncert13)      

f=open("delaylog.dat","a")
f.write("%f %f %f %f %f %f %f %f %f\n" % (startTime, ha, dec, delay12, uncert12, delay23, uncert23, delay13, uncert13))
