import numpy
import matplotlib
import matplotlib.pyplot as plt

data = numpy.loadtxt('visplot.dat').transpose()


x=data[0]

ylabelx = -0.2
plt.rc('legend',fontsize=10)

ax1 = plt.subplot(1,2,1)
plt.title("Quiet morning")
ax1.yaxis.set_label_coords(ylabelx,0.5)
plt.setp(ax1.get_xticklabels(),visible=False)
plt.plot(x,data[1],'r',label='1-3')
plt.plot(x,data[4],'b',label='3-6')
plt.plot(x,data[7],'g',label='1-6')
plt.legend(title='Baseline')
plt.ylabel('Amplitude, psJy')
plt.xlim(30,53)

ax4 = plt.subplot(1,2,2,sharey=ax1)
plt.title("Media crews arrived")
plt.setp(ax4.get_xticklabels(),visible=False)
plt.setp(ax4.get_yticklabels(),visible=False)
plt.plot(x,data[1],'r')
plt.plot(x,data[4],'b')
plt.plot(x,data[7],'g')
plt.ylim(0,2900)
plt.xlim(131,160)


plt.gcf().subplots_adjust(hspace=0,wspace=0)
#plt.show()
#plt.savefig("out.eps",transparent=True)
plt.savefig("out.png",transparent=False)
