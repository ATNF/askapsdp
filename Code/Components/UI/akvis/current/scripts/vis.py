
class SummaryPlotter(object):
    def __init__(self, nant, nbeams, npol, nchan, start_freq, chanbw):
        self._nant = nant
        self._nbeams = nbeams
        self._npol = npol
        self._nchan = nchan
        self._sum = corr_summary.Summariser(nant, nbeams, npol, nchan, start_freq, chanbw)
        self._nbl = nant * (nant - 1) / 2
        self._nhist = 100
        n = self._nhist
        self._amps = np.zeros((n, self._nbl, nbeams), dtype=np.float) +1 
        self._delays = np.zeros((n, self._nbl, nbeams), dtype=np.float) +1 
        self._phases = np.zeros((n, self._nbl, nbeams), dtype=np.float) +3 
        self._times = np.zeros((n, self._nbl, nbeams), dtype=np.float) +4 
        self._indexes = np.zeros((self._nbl, nbeams), dtype=np.int) +5
        self._curr_beam = 0
        self._amp_lines = []
        self._phase_lines = []
        self._delay_lines = []

        self._fig = pylab.figure(2)
        self.replot()


    def put_cross(self, blidx, beam, pol, spectrum):
        i = self._indexes[blidx, beam]
        if pol == 0:
            popt, pcov = self._sum.get_apd(spectrum)
            amp, delay, phase = popt
            self._amps[i, blidx, beam] = amp
            self._phases[i, blidx, beam] = phase
            self._delays[i, blidx, beam] = delay
            self._times[i, blidx, beam] = time.time()
            self._indexes[blidx, beam] =  (self._indexes[blidx, beam] + 1)  % self._nhist
            print "SUM", blidx, beam, pol, amp, delay, phase

    def replot(self):
        t = self._times[:, :, self._curr_beam]
        a = self._amps[:, :, self._curr_beam]
        p = self._phases[:, :, self._curr_beam]
        d = self._delays[:, :, self._curr_beam]
        self._fig = pylab.figure(2)
        ax1 = pylab.subplot(3,1,1)
        self._amp_lines = pylab.plot(t, a)
        ax2 = pylab.subplot(3,1,2)
        self._phase_lines = pylab.plot(t, p)
        ax3 = pylab.subplot(3,1,3)
        self._delay_lines = pylab.plot(t, d)
        self._axes = [ax1, ax2, ax3]
        # 1 line for each baseline i.e. for 15 baselines # polarisations not supported yet
        self._lines = [self._amp_lines, self._phase_lines, self._delay_lines]
        self._ydata = [self._amps, self._delays, self._phases]
        self._xdata = self._times
        self.redraw()
    
    def redraw(self):
        """ Adds a redraw to the idle queue"""
        queue_idle(self._timed_draw, self._fig)

    def set_line_data(self):
        t = Timer()
        t.start()
        for lines, ax, ydata in zip(self._lines, self._axes, self._ydata): # For each subplot
            for iline, line in enumerate(lines):
                y = ydata[:, iline, self._curr_beam]
                x = self._xdata[:, iline, self._curr_beam]
                x = np.arange(iline, iline+10)
                y = x**iline
                line.set_ydata(y)
                line.set_xdata(x)

        t.stop()

        
    def _timed_draw(self):
        with Timer() as t:
            pylab.draw()

        self._draw_timer = t
        
    def update_data(self):
#        self.set_line_ydata()
#        self._update_axes_ylim()
#        self.redraw()
#        self.replot() # slow
        self.set_line_data()

