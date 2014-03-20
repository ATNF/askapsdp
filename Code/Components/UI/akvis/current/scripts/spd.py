#!/usr/bin/env python
"""
A class that plots visibility spetra like SPD does at the ATCA


AAARGH:
For some reason 2 plot windows doens't work - TK bug no doubt.

Copyright (C) Keith Bannister 2014
"""
import pylab
import matplotlib
import numpy as np
import os
import sys
import logging
from askap.akvis.testutils import Timer
import threading
import time
import re
import matplotlib.ticker as ticker
from askap.akvis import corr_summary
import zmq
import struct
#__version__ = '0.1'

POL_STR = ('XX','XY','YX','YY')

def printstack():
    import sys
    import traceback
    print >> sys.stderr, "\n*** STACKTRACE - START ***\n"
    code = []
    for threadId, stack in sys._current_frames().items():
        code.append("\n# ThreadID: %s" % threadId)
        for filename, lineno, name, line in traceback.extract_stack(stack):
            code.append('File: "%s", line %d, in %s' % (filename,
                                                    lineno, name))
            if line:
                code.append("  %s" % (line.strip()))
                
    for line in code:
        print >> sys.stderr, line

    print >> sys.stderr, "\n*** STACKTRACE - END ***\n"


def sig_printstack(signal, frame):
    print "GOT SIGNAL", signal, frame
    printstack()

import signal
signal.signal(signal.SIGUSR1, sig_printstack)

#print 'i am', os.getpid()

def myafter(self, ms, func, *args):
    # Have to do a heanous fix to a bug on my mac OSX where the call to 
    # either _register() or tk.call('after') blocks if the window doesn't have
    # the focus.
    # adding the sleep fixes this.
    """Call function once after given time.                                                   
    
    MS specifies the time in milliseconds. FUNC gives the                                     
    function which shall be called. Additional parameters                                     
    are given as parameters to the function call.  Return                                     
    identifier to cancel scheduling with after_cancel."""
    if not func:
        # I'd rather use time.sleep(ms*0.001)                                                 
        self.tk.call('after', ms)
    else:
        def callit():
            try:
                func(*args)
            finally:
                try:
                    self.deletecommand(name)
                except TclError:
                    pass

    name = self._register(callit)
    time.sleep(0.05) # AAARGH Why do I have to do this?
    v = self.tk.call('after', ms, name)

    return v

def queue_idle(cmd, fig):
    backend = matplotlib.get_backend()
    if backend == 'TkAgg':
        win = fig.canvas.manager.window
#        myafter(win, 'idle', cmd) # Tell TkAgg to update the attached command when idle
        win = fig.canvas.draw_idle()
    else:
        assert False, 'Unknown backend %s'  % backend


def bl2idx(bl, nant):
    """
    >>> bl2idx((1, 2), 6)
    0
    >>> bl2idx((1, 3), 6)
    1
    >>> bl2idx((1, 4), 6)
    2
    >>> bl2idx((1, 5), 6)
    3
    >>> bl2idx((1, 6), 6)
    4
    >>> bl2idx((1, 2), 6)
    0
    >>> bl2idx((2, 1), 6)
    0
    >>> bl2idx((5, 6), 6)
    15
    
    """
    a1, a2 = bl

    assert a1 >= 0 and a1 <= nant
    assert a2 >= 0 and a2 <= nant
    assert a1 != a2

    # bl 1-2 is the same as 2-1
    if a1 > a2:
        a1, a2 = a2, a1

    idx = ((a1 - 1) +  nant*(a2 - 1 - 1)) / 2
    return idx
    

def idx2bl(idx, nant):
    """
    >>> nant = 6
    >>> idx2bl(0, nant)
    (1, 1)
    >>> idx2bl(1, nant)
    (1, 2)
    >>> idx2bl(7, nant)
    (2, 1)
    """
    assert a1 >= 0 and a1 <= nant
    assert a2 >= 0 and a2 <= nant

    a1 = idx / nant
    a2 = idx % nant
    bl = (a1+1, a2+1)
    return bl
    

class SpectrumPlotter(object):
    def __init__(self, nant, nbeams, npol, nchan):
        self._nant = nant
        self._nbl = nant * (nant - 1)/2
        self._nbeams = nbeams
        self._npol = npol
        self._nchan = nchan
        self._cross = np.zeros((self._nbl, nbeams, npol, nchan), dtype=np.complex)
        self._num_puts = 0
        self._prev_cross = None
        self._auto = np.zeros((nant, nbeams, npol, nchan))
        self._ant_mask = np.ones(nant, dtype=np.bool)
        self._pol_mask = np.ones(npol, dtype=np.bool)
        self._curr_beam = 0
        self._lines = []
        self._fig = pylab.figure(1)
        self._varfunc = {'phase': lambda v, p: np.degrees(np.angle(v)), 
                        'amp':lambda v, p: abs(v),
                        'real':lambda v, p: np.real(v),
                         'imag': lambda v, p: np.imag(v),
                         'diff': lambda v, p: abs(v - p),
                         'quot': lambda v, p: abs(v)/abs(p)}

        self._var_ylim = {'phase':None, 'amp':None, 'real':None, 'imag': None, 'diff':None, 'quot':None}
        self._variable = 'phase'
        self._do_averaging = False
        self._rsave = False
        self._navg = 0
        self._nxy = (3, 5)
        self._draw_timer = Timer()
        self.replot()
        

    def set_variable(self, var):
        """Set the variabel that is plotted.
        var can be:
        'phase'
        'amp'
        'real'
        'imag'

        """
        assert var in self._varfunc.keys(), 'Unknown variable requested:%s'%var
        if (var == 'diff' or var == 'quot') and self._prev_cross is None:
            raise ValueError('No saved data for diff or quot. Use rsave or save')

        self._variable = var
        logging.debug('settign variable: %s %s', var, self._variable)
        
        self.update_data()
        

    def _update_axes_ylim(self):
        ylim = self._var_ylim[self._variable]
        self._update_axes_limits(ylim, 'y')

    def _update_axes_limits(self, limits, axis):

        assert axis == 'x' or axis == 'y'

        for ax in self._axes:
            if limits is None:
                ax.relim()
                ax.autoscale(True, axis)

            else:
                if axis == 'y':
                    ax.set_ylim(limits)
                else:
                    ax.set_xlim(limits)

    def set_ylim(self, ylim):
        """ Applies to the current variable"""
        self._var_ylim[self._variable] = ylim
        logging.debug('Setting ylim %s for var %s', ylim, self._variable)
        self._update_axes_ylim()
        self.redraw()
        
    def set_channel_range(self, xlim):
        self._xlim = xlim
        logging.debug('Setting xlim %s' % xlim)
        self._update_axes_limits(self._xlim, 'x')
        self.redraw()

    def set_averaging(self, do_averaging):
        self._navg = np.ones((self._nbl, self._nbeams, self._npol), dtype=int)
        self._do_averaging = bool(do_averaging)


    def save(self):
        if self._prev_cross is None: # lazy initialise
            self._prev_cross = np.empty_like(self._cross)

        self._prev_cross[:] = self._cross[:]
        print 'Data saved', self._prev_cross.shape

    def set_rsave(self, rsave):
        if rsave:
            self.save()

        self._rsave = rsave

    def set_layout(self, nxy):
        self._nxy = nxy
        self.replot() # need ot replot for this as all the axes will be all over the place!

    def savefig(self, fileout):
        pylab.savefig(fileout)
        nbytes = os.path.getsize(fileout)
        print 'Saved file to ', fileout, '(%d kbytes)'%(nbytes/1024)


    def print_stats(self):
        cross_shape = self._cross.shape
        redraw_time = str(self._draw_timer)
        num_puts = self._num_puts
        var = self._variable
        beam = self._curr_beam
        avg = self._do_averaging
        navg = self._navg
        polstr = " ".join([pol for ipol, pol in enumerate(POL_STR) if self._pol_mask[ipol]])
        
        s = """
Shape: %(cross_shape)s Num puts: %(num_puts)s Redraw time: %(redraw_time)s
Variable: %(var)s Beam: %(beam)s Polarisations: %(polstr)s
Averaging %(avg)s Navg: %(navg)s
""" % locals()
        for var, ylim in self._var_ylim.iteritems():
            if ylim is None:
                ylim = 'auto'

            s += "Var %s\tylim: %s\n" % (var, ylim)
            
        print s
                

    def _get_var(self, ibl, ipol):
        vfunc = self._varfunc[self._variable]
        v = self._cross[ibl, self._curr_beam, ipol, :]
        p = None
        ps = None
        pcs = None
        if self._prev_cross is not None:
            p = self._prev_cross[ibl, self._curr_beam, ipol, :]
            pcs = self._prev_cross.shape
            ps = p.shape
            
        d = vfunc(v, p)
        return d

    def replot(self):
        nant = sum(self._ant_mask)
        nant = self._nant
        nbl = nant * (nant - 1) / 2
        nrows = int(np.sqrt(nbl)) 
        ncols = int(np.sqrt(nbl)) + 1
        nrows, ncols= self._nxy
        fig = self._fig
        self._lines = []
        self._axes = []
        for ibl in xrange(self._nbl):
            ax = pylab.subplot(nrows, ncols, ibl+1)
            self._axes.append(ax)
            pylab.cla()
            bllines = []
            self._lines.append(bllines)
            for p in xrange(4): # polarisations
                d = self._get_var(ibl, p)
                line, = pylab.plot(d)
                bllines.append(line)

            ax.get_xaxis().set_major_locator(matplotlib.ticker.MaxNLocator(4))

        pylab.subplots_adjust(left=0.05, right=0.95, top=0.95, bottom=0.05)
        self.redraw()

    def set_line_ydata(self):
        t = Timer()
        t.start()
        vfunc = self._varfunc[self._variable]
        ylim = self._var_ylim[self._variable]
        for ibl, (bllines, ax) in enumerate(zip(self._lines, self._axes)):
            for p,line in enumerate(bllines):
                d = self._get_var(ibl, p)
                line.set_ydata(d)


        t.stop()

    def set_polarisation_mask(self, pol_mask):
        assert len(pol_mask) == len(self._pol_mask)
        self._pol_mask = pol_mask
        for bllines in self._lines:
            for vis, pol_line in zip(pol_mask, bllines):
                pol_line.set_visible(vis)

        self.redraw()

    def set_antenna_mask(self):
        raise NotImplemented('Havent got there yet. Baseline numbering is too hard for me')

    def set_beam(self, beamno):
        assert beamno >=0 and beamno < self._nbeams, 'Invalid beam %s' % beamno
        self._curr_beam = beamno
        self.update_data()
        
    def update_data(self):
        #self._queue_idle(self.set_line_data)
        self.set_line_ydata()
        self._update_axes_ylim()


    def redraw(self):
        """ Adds a redraw to the idle queue"""
        queue_idle(self._timed_draw, self._fig)
        
    def _timed_draw(self):
        with Timer() as t:
            pylab.draw()

        print "DRAW TIMER", t, self._draw_timer
        self._draw_timer = t



    def put_cross(self, blidx, beam, pol, spectrum):

        if self._rsave:
            self._prev_cross[blidx, beam, pol, :] = self._cross[blidx, beam, pol, :]

        if self._do_averaging:
            n = float(self._navg[blidx, beam, pol])
            # Update average in place
            self._cross[blidx, beam, pol, :] = (self._cross[blidx, beam, pol, :] * n + spectrum)/(n+1.) 
            self._navg[blidx, beam, pol] += 1
        else:
            self._cross[blidx, beam, pol, :] = spectrum

        self._num_puts += 1


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


def complex_noise(amp, n):
    noise = np.random.randn(n)*amp + 1j*np.random.randn(n)*amp
    return noise

class TestDataGenerator(object):
    def __init__(self, nant, nbeams, npol, nchan):
        self._nant = nant
        self._nbeams = nbeams
        self._npol = npol
        self._nchan = nchan
        self._simstep = 0
        self._noise_amp = 0.1

    def _gen_noise(self):
        return complex_noise(self._noise_amp, self._nchan)
        

    def push(self, plotters, delay_step=1.):
        t = Timer()
        t.start()
        nant = self._nant
        nbl = nant * (nant - 1)/2
#        print 'Simstep', self._simstep
        channels = np.arange(self._nchan)/float(self._nchan)
        nspec = np.zeros(self._nchan, dtype=np.complex)
        for bl in xrange(nbl):
            delay = np.sin(self._simstep*delay_step)*bl
#            print 'PUSH', 'BL', bl, 'delay', delay
            phase = 0.1
            phases = 1j*(2*np.pi*channels*delay + phase)
            ispec = np.exp(phases)

            for beam in xrange(self._nbeams):
                for p in plotters:
                    p.put_cross(bl, beam, 0, ispec + self._gen_noise()) # stokes I
                for i in xrange(1, 4):
                    for p in plotters:
                        p.put_cross(bl, beam, i, nspec + self._gen_noise()) # stokes Q, U, V

                 
        t.stop()
#        print 'Gen data took', str(t)

        t = Timer()
        with t:
            for p in plotters:
                p.update_data()
                p.redraw() # vis REDRAW complains with RuntimeError: main thread is not in main loop. No idea why.

            plotters[0].redraw()

#        print 'Update data took', str(t)
        
        self._simstep += 1

class ZmqDataGenerator(object):
    def __init__(self, target):
        self.target = target
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.SUB)
        self.socket.connect(target)
        self.subscriptions = []

    def get_sockstring(self, beam, pol):
        assert pol.upper() in POL_STR
        assert beam >= 0
        s = '%d%s' % (beam, pol.upper())
        return s

    def subscribe(self, beam, pol):
        s = self.get_sockstring(beam, pol)
        assert s not in self.subscriptions
        self.socket.setsockopt_string(zmq.SUBSCRIBE, s.decode('ascii'))
        self.subscriptions.append(s)

    def unsubscribe(self, beam, pol):
        s = self.get_sockstring(beam, pol)
        assert s in self.subscriptions
        self.socket.setsockopt_string(zmq.UNSUBSCRIBE, s.decode('ascii'))
        self.subscriptions.remove(s)

    def unsubscribe_all(self):
        for s in self.subscriptions:
            self.socket.setsockopt_string(zmq.UNSUBSCRIBE, s.decode('ascii'))

        self.subscriptions = []

    def push(self, plotters):
        msg = self.socket.recv_multipart()
        vmsg = VisabilityMessage(msg)
        print "msg RX", msg[0], str(vmsg)

        t = Timer()
        t.start()
        nbl = vmsg.nBaselines # this includes autocorelations
        nbl -= 6
        
        for bl in xrange(nbl):
            for p in plotters:
                p.put_cross(bl, vmsg.beamId, vmsg.polarisationId, vmsg.visibilities[bl, :])
        t.stop()

        t = Timer()
        with t:
            for p in plotters:
                p.update_data()
                p.redraw() # vis REDRAW complains with RuntimeError: main thread is not in main loop. No idea why.

            plotters[0].redraw()


class BufferReader(object):
    def __init__(self, bytestr):
        self.data_bytes = bytearray(bytestr)
        self.off = 0

    def read_double(self, n=1):
        d = np.frombuffer(self.data_bytes, dtype=np.double, offset=self.off, count=n)
        self.off += n*8
        return d

    def read_uint32(self, n=1):
        d = np.frombuffer(self.data_bytes, dtype=np.uint32, offset=self.off, count=n)
        self.off += n*4
        return d

    def read_uint64(self, n=1):
        d = np.frombuffer(self.data_bytes, dtype=np.uint64, offset=self.off, count=n)
        self.off += n*8
        return d

    def read_complex64(self, n=1):
        d = np.frombuffer(self.data_bytes, dtype=np.complex64, offset=self.off, count=n)
        self.off += n*8
        return d

    def read_uint8(self, n=1):
        d = np.frombuffer(self.data_bytes, dtype=np.uint8, offset=self.off, count=n)
        self.off += n
        return d

class VisabilityMessage(object):
    def __init__(self, msgs):
        self.pol_ident = msgs[0]
        r = BufferReader(msgs[1])
        self.timestamp = r.read_uint64()
        self.beamId = r.read_uint32()
        self.polarisationId = r.read_uint32()
        self.nChannels = r.read_uint32()
        self.chanWidth = r.read_double()
        self.frequency = r.read_double(self.nChannels)
        self.nBaselines = r.read_uint32()
        self.antenna1 = r.read_uint32(self.nBaselines)
        self.antenna2 = r.read_uint32(self.nBaselines)
        self.visibilities = r.read_complex64(self.nBaselines*self.nChannels)
        self.visibilities.shape = (self.nBaselines, self.nChannels)
        self.flag = r.read_uint8(self.nBaselines*self.nChannels)
        self.flag.shape = (self.nBaselines, self.nChannels)

    def __str__(self):
        s = "t=%d beam=%d pol=%d chanWidth=%0.1f kHz Nbl: %d vishape: %s" % (self.timestamp, 
               self.beamId, 
               self.polarisationId,
               self.chanWidth,
               self.nBaselines,
               str(self.visibilities.shape))

        return s
        

def push_data(plt, vis, gen):
    import time
    time.sleep(1)
    plotters = [plt]
    if vis is not None:
        plotters.append(vis)

    while True:
        t = Timer()
        with t:
            gen.push(plotters)
#        print 'push took', str(t)



sel_regex = re.compile(r'sel\s*(.*)|sel\s*(b)(\d+)')
chan_regex = re.compile(r'(\d+)[\s+:](\d+)\s*')
scale_regex = re.compile(r'scale\s*((\d+\.?\d*)\s*(\d+\.?\d*))?')

def parse_command(cmd, plt, gen):
    import Tkinter
    bits = cmd.split()
    scalem = scale_regex.match(cmd)
    selm = sel_regex.match(cmd)


    if cmd == 'quit':
        print 'Quitting'
        Tkinter.tkinter.quit()
        sys.exit(0)
    elif cmd == '':
        pass
    elif cmd == 'a':
        plt.set_variable('amp')
    elif cmd == 'p':
        plt.set_variable('phase')
    elif cmd == 'r':
        plt.set_variable('real')
    elif cmd == 'i':
        plt.set_variable('imag')
    elif cmd == 'd':
        plt.set_variable('diff')
    elif cmd == 'q':
        plt.set_variable('quot')
    elif cmd == 'replot':
        plt.replot()
    elif selm is not None:
        if selm.group(1) is not None:
            pol = selm.group(1)
            pol_mask = [p.lower() in pol for p in POL_STR]
            plt.set_polarisation_mask(pol_mask)
            gen.unsubscribe_all()
            for p in pol.split():
                gen.subscribe(plt._curr_beam, p)
            
        elif selm.group(2) == 'b':
            beamno = int(selm.group(3))
            plt.set_beam(beamno)
            gen.unsubscribe_all()
            pols = [POL_STR[i] for i in xrange(len(POL_STR)) if plt._pol_mask[i]]
            for p in pols:
                gen.subscribe(beamno, p)
        else:
            print 'Unknown select command %s' % cmd
            return

    elif cmd.startswith('chan'):
        if len(bits) == 1:
            plt.set_channel_range(None)
        elif len(bits) == 3:
            xlim = map(int, bits[1:])
            plt.set_channel_range(xlim)
    
    elif cmd.startswith('scale'):
        if len(bits) == 1:
            plt.set_ylim(None)# autoscale
        elif len(bits) == 3:
            try:
                ylim = map(float, bits[1:])
                plt.set_ylim(ylim)
            except:
                print 'Invalid ylim cmd %s'%s
                return

    elif cmd.startswith('avg'):
        plt.set_averaging(True)
    elif cmd.startswith('noavg'):
        plt.set_averaging(False)
    elif cmd.startswith('rsave'):
        plt.set_rsave(True)
    elif cmd.startswith('norsave'):
        plt.set_rsave(False)
    elif cmd.startswith('save'):
        plt.save()
    elif cmd.startswith('pstat'):
        plt.print_stats()
    elif cmd.startswith('layout'):
        if len(bits) == 3:
            try:
                ncols, nrows = map(int, bits[1:])
                plt.set_layout((nrows, ncols))
            except:
                print 'Invalid layout command %s' % cmd
        else:
                print 'Invalid layout command %s' % cmd
                
    elif cmd.startswith('write'):
        bits = cmd.split()
        if len(bits) == 2:
            fout = bits[1]
            plt.savefig(fout)
        else:
            print 'Invalid write command %s' % cmd
            
    elif cmd.startswith('array'):
        if len(bits) == 2:
            a = bits[1]
            arr_mask = [str(i) in a for i in xrange(1, plt.nant+1)]
            plt.set_antenna_mask(arr_mask)

    elif cmd.startswith('?') or cmd.startswith('help'):
        print_help()
    else:
        print 'Unknown command: %s' % cmd

def print_help():
    version = __version__
    s = """SPD for ASKAP %(version)s (C) CSIRO 2014
--- Variable selection ---
a = amplitudes
p = phases
r = real
i = imaginary
d = difference between current and saved = abs(curr - saved)
q = quotition of current and saved = abs(curr)/abs(saved)

--- Data selection ---
sel i|q|u|v = select stokes IQUV
sel bN = select beam N
scale [start end] = set y scale between start and end. If not specified, autoscale
chan [start end] = plot channels from start to end. If not specified, autoscale

--- Previous buffer ---
save - Save current data to previous buffer, so you can plot with 'd' or 'q'
rsave - Save new data to previous buffer, so  you can plot with 'd' or 'q'
norsave - Stop saving new data to previous buffer

--- Averaging ---
avg - Start averaging
noavg - Stop averaging

--- Miscellaneous ---
pstat - print plotting statistics
write FILE - Save figure to file: Supported formats from extension (e.g. spec.png, spec.pdf)
layout X Y - Make figure X subfigs wide and Y subfigs tall
replot - Replot the figure.
help - print this out
    """ % locals()
    print s


def input_loop(plt, vis, gen):
    import readline
    import sys
    import Tkinter
    while True:
        # Don't use raw_input, it breaks tk inter
        print 'SPD>',
        cmd = sys.stdin.readline().strip()
        try:
            parse_command(cmd, plt, gen)
        except:
            logging.info('Error running command', exc_info=True)


def _main():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Script description')
    parser.add_argument('-v', '--verbose', dest='verbose', action='store_true', help='Be verbose [Default %default]')
    parser.add_argument(dest='infiles', nargs='*', help='Input data files (none for testing)')
    parser.set_defaults(verbose=False)
    values = parser.parse_args()
    if values.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)

    nant = 6;     nbeams = 1;     nchan = 16416
    #nant = 6; nbeams = 9 ; nchan = 16384
#    nchan = 304

    npol = 4

    start_freq = 1.4 # Ghz
    chanbw = 1e-3 # Ghz
    plt = SpectrumPlotter(nant, nbeams, npol, nchan)
#    vis = SummaryPlotter(nant, nbeams, npol, nchan, start_freq, chanbw)
    vis = None

    print values.infiles
    if values.infiles is None or len(values.infiles) == 0:
        gen = TestDataGenerator(nant, nbeams, npol, nchan)
    else:
        gen = ZmqDataGenerator(values.infiles[0])
    
    thread = threading.Thread(target=push_data, args=(plt, vis, gen))
    thread.daemon = True
    thread.start()
#    push_data(plt, gen)

    input_thread = threading.Thread(target=input_loop, args=(plt, vis, gen))
    input_thread.daemon = True
    input_thread.start()

    pylab.show()

if __name__ == '__main__':
    _main()
