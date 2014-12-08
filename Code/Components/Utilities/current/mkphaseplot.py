# helper tool to analyse phase spectra which go into delay solver. It uses the output of delaysolver 
# (file avgspectrum.dat). It is intended for debugging only and requires matplotlib and numpy to be installed
# checked in to our code tree for convenience
#
import numpy
import matplotlib
import matplotlib.pyplot as plt

data = numpy.loadtxt('avgspectrum.dat').transpose()

ylabelx = -0.07
plt.rc('legend',fontsize=10)
counter = 1
antenna_names = ['AK06','AK01','AK03','AK15','AK08','AK09']

xaxes = [None,None,None]
yaxes = [None,None,None,None,None]

def plotBaseline(counter,data, ant1, ant2):
   xindex = (counter - 1) % 3
   yindex = (counter - 1) / 3
   ax = plt.subplot(5,3,counter, sharex=xaxes[xindex], sharey=yaxes[yindex])
   if xaxes[xindex] == None:
      xaxes[xindex] = ax
   plt.xlim(-1,305)
   if yaxes[yindex] == None:
      yaxes[yindex] = ax
   plt.ylim(-199,199)
   ax.locator_params(tight=True, nbins=5)
   ax.yaxis.set_label_coords(ylabelx,0.2)
   if xindex == 0:
      if yindex == 2:
         plt.ylabel("   Phase, deg")
         plt.setp(ax.get_yticklabels(),visible=True)
   else:
      plt.setp(ax.get_yticklabels(),visible=False)
   if yindex == 4:
      if xindex == 1:
         plt.xlabel("Channel")
         plt.setp(ax.get_xticklabels(),visible=True)
   else:
      plt.setp(ax.get_xticklabels(),visible=False)
   xdata = [data[2,i] for i in range(len(data[2,:])) if data[0,i] == ant1 and data[1,i] == ant2]
   ydata = [data[3,i] for i in range(len(data[3,:])) if data[0,i] == ant1 and data[1,i] == ant2]
   if len(xdata) == 0 or len(ydata) == 0:
      plt.text(100,-20,"excluded")
   plt.plot(xdata,ydata,"r", label="%s-%s" % (antenna_names[ant1],antenna_names[ant2]))  
   plt.text(160,110,"%s-%s" % (antenna_names[ant1],antenna_names[ant2]))  
   #plt.legend()

for ant1 in range(6):
   for ant2 in range(ant1,6):
       if ant1 == ant2:
          continue
       plotBaseline(counter, data,ant1,ant2)
       counter = counter + 1
 
plt.gcf().subplots_adjust(hspace=0,wspace=0)
plt.savefig("out.png",transparent=False)

