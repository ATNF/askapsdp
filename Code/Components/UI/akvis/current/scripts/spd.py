#!/usr/bin/env python
"""
A class that plots visibility spetra like SPD does at the ATCA


AAARGH:
For some reason 2 plot windows doens't work - TK bug no doubt.

Copyright (C) Keith Bannister 2014
"""
import matplotlib
import pylab
import numpy as np
import os
import sys
import logging
from askap.akvis.testutils import Timer
from askap.time import bat2utcDt
import threading
import time
import re
import matplotlib.ticker as ticker
from askap.akvis import corr_summary
import zmq
import struct

POL_STR = ('XX','XY','YX','YY')
ANTENNA_LABELS = (6, 1, 3, 15, 8, 9)
PLOT_MODES = 'abp' # (antenna, beam, polarisation)

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
        fig.canvas.draw_idle()

#        myafter(win, 'idle', cmd) # Tell TkAgg to update the attached command when ide
    else:
        assert False, 'Unknown backend %s'  % backend
    
class SpectrumPlotter(object):
    def __init__(self, dgen, nant, nbeams, npol, freq, antenna1=None, antenna2=None):
        self._nant = nant
        self._nbl = nant * (nant + 1)/2
        self._nbeams = nbeams
        self._npol = npol
        self._nchan = len(freq)
        self._cross = np.zeros((self._nbl, nbeams, npol, self._nchan), dtype=np.complex)
        self._flags = np.zeros((self._nbl, self._nchan), dtype=np.bool)
        self._num_puts = 0
        self._prev_cross = None
        self._lines = []
        self._fig = pylab.figure(1)
        self._mgr = SubscriptionManager(dgen, nant, npol, nbeams)
        self._varfunc = {'phase': lambda v, p: np.degrees(np.angle(v)), 
                        'amp':lambda v, p: abs(v),
                         'dba': lambda v, p: 10*np.log10(abs(v)),
                        'real':lambda v, p: np.real(v),
                         'imag': lambda v, p: np.imag(v),
                         'diff': lambda v, p: abs(v - p),
                         'quot': lambda v, p: abs(v)/abs(p)}

        self._var_ylim = {'phase':None, 'amp':None, 'real':None, 'imag': None, 'diff':None, 'quot':None, 'dba': None}
        self._variable = 'phase'
        self._do_averaging = False
        self._rsave = False
        self._navg = 0
        self._nxy = (3, 5)
        self._draw_timer = Timer()
        self._stack = 'p'
        self._panel = 'a'
        self._plot_autos = False
        self._xmode = 'freq'
        self._xdata = self._mgr.dgen.frequency
        self.baselines_ant1 = antenna1
        self.baselines_ant2 = antenna2
        self._legend_labels = []
        self._legend_lines = []
        self._legend_on = False
        self._figlegend = None
        self._mask_flagged = True

        self.replot()

    def set_legend(self, legon):
        self._legend_on = bool(legon)
        self._draw_legend()
        self.redraw()

    def set_flag(self, flag):
        self._mask_flagged = flag
        self.update_data()
        self.redraw()

    def set_grid(self, gridon):
        for ax in self._axes:
            ax.grid(gridon)

        self.redraw()

    def _draw_legend(self):
        if self._legend_on:
            pylab.subplots_adjust(left=0.05, right=0.90, top=0.95, bottom=0.05)
            lines_only = [line for (line, slice_list) in self._legend_lines]
            self._figlegend = pylab.figlegend(lines_only, self._legend_labels, loc='upper right')
        else:
            pylab.subplots_adjust(left=0.05, right=0.95, top=0.95, bottom=0.05)
            if self._figlegend is not None:
                self._figlegend.set_visible(False)

    def set_panel(self, pan):
        assert pan in PLOT_MODES
        self._panel = pan
        self.replot()

    def set_stack(self, stack):
        print 'stack', stack, PLOT_MODES
        assert stack in PLOT_MODES
        self._stack = stack
        self.replot()

    def toggle_xmode(self):
        if self._xmode == 'freq':
            self.set_xmode('chan')
        else:
            self.set_xmode('freq')

    def set_xmode(self, xmode):
        assert xmode in ('freq','chan')

        if xmode == 'freq':
            self._xdata = self._mgr.dgen.frequency
        elif xmode == 'chan':
            self._xdata = np.arange(len(self._mgr.dgen.frequency))

        self._xmode = xmode
        self.set_line_xdata()
        self._update_axes_limits(None, 'x')
        self.redraw()
        
    def set_variable(self, var):
        """Set the variable that is plotted.
        var can be:
        'phase'
        'amp'
        'real'
        'imag'
        'dba'

        """
        assert var in self._varfunc.keys(), 'Unknown variable requested:%s'%var
        if (var == 'diff' or var == 'quot') and self._prev_cross is None:
            raise ValueError('No saved data for diff or quot. Use rsave or save')

        self._variable = var
        logging.debug('settign variable: %s %s', var, self._variable)
        
        self.update_data()
        self.redraw()
        

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
        beam = np.arange(len(self._mgr.beam_mask))[self._mgr.beam_mask]
        avg = self._do_averaging
        navg = self._navg
        panel = self._panel
        stack = self._stack
        mask_flagged = self._mask_flagged
        antennas = np.arange(len(self._mgr.ant_mask))[self._mgr.ant_mask]
        polstr = " ".join([pol for ipol, pol in enumerate(POL_STR) if self._mgr.pol_mask[ipol]])
        
        s = """
Shape: %(cross_shape)s Num puts: %(num_puts)s
Variable: %(var)s 
Beams: %(beam)s
Polarisations: %(polstr)s
Antennas: %(antennas)s
Averaging %(avg)s Navg: %(navg)s Flagging: %(mask_flagged)s
Panel: %(panel)s Stack: %(stack)s
""" % locals()
        for var, ylim in self._var_ylim.iteritems():
            if ylim is None:
                ylim = 'auto'

            s += "Var %s\tylim: %s\n" % (var, ylim)
            
        print s
                

    def _get_var(self, slice_list):
        vfunc = self._varfunc[self._variable]
        v = self._cross[slice_list]
        # flag shape is (nBaselines, Nchannels)
        # data shape is (Nbaselines, nBeam, nPol, Nchan)
        flag_slice_list = [slice_list[0], slice_list[3]]
        flags = self._flags[flag_slice_list]
        if self._mask_flagged:
            v_ma = np.ma.masked_array(v, flags)
        else:
            v_ma = v

        p = None
        p_ma = None
        if self._prev_cross is not None:
            p = self._prev_cross[slice_list]
            if self._mask_flagged:
                p_ma = np.ma.masked_array(p, flags)
            else:
                p_ma = p
    
        d = vfunc(v_ma, p_ma)
        return d

    def replot(self):
        nrows, ncols= self._nxy
        fig = self._fig
        self._lines = []
        self._axes = []
        npanels = nrows * ncols
        pylab.clf()
        for ipan, (pan_dimidx, pan_vidx, pan_label) in \
                enumerate(self._mgr.mode_iter(self._panel)):

            if nrows * ncols == ipan:
                # too many things to plot - nxy needs to be made larger
                break

            ax = pylab.subplot(nrows, ncols, ipan+1)
            self._axes.append(ax)
            pylab.cla()
            bllines = []
            self._lines.append(bllines)
            if ipan == 0:
                self._legend_lines = bllines
                self._legend_labels = []

            for istack, (st_dimidx, st_vidx, st_label) in \
                    enumerate(self._mgr.mode_iter(self._stack)):
                # make a slice list - containing slices for each of the 4 axes
                # which are [baseline, beam, pol, chan]
                # always choose all channels
#                print 'bl mask', self._mgr.baseline_mask, np.nonzero(self._mgr.baseline_mask)
#                print 'beam mask', self._mgr.beam_mask, np.nonzero(self._mgr.beam_mask)
#                print 'pol_mask', self._mgr.pol_mask, np.nonzero(self._mgr.pol_mask)
                slice_list = [np.nonzero(self._mgr.baseline_mask)[0][0],
                              np.nonzero(self._mgr.beam_mask)[0][0],
                              np.nonzero(self._mgr.pol_mask)[0][0],
                              slice(None)]

#                print 'orig slice list', slice_list, pan_dimidx, pan_vidx, st_dimidx, st_vidx
                slice_list[pan_dimidx] = pan_vidx
                slice_list[st_dimidx] = st_vidx

                d = self._get_var(slice_list)
                # transpose if required
                dflat = d.flatten()

 #               print 'Xdata shape', self._xdata.shape, \
 #                   'Ydata shape', d.shape, dflat.shape, \
 #                   'slice list', slice_list, \
 #                   'pan label', pan_label, ipan, 'st_label', st_label
                
                if ipan == 0:
                    self._legend_labels.append(st_label)

                line, = pylab.step(self._xdata, dflat, label=st_label)
                bllines.append((line, slice_list))

            ax.get_xaxis().set_major_locator(matplotlib.ticker.MaxNLocator(4))
            pylab.title(pan_label)

            
        self._draw_legend()
        self.redraw()

    def set_line_ydata(self):
        t = Timer()
        t.start()
        for ibl, (bllines, ax) in enumerate(zip(self._lines, self._axes)):
            for p,(line, slice_list) in enumerate(bllines):
                d = self._get_var(slice_list)
                line.set_ydata(d)

        t.stop()

    def set_line_xdata(self):
        for ibl, (bllines, ax) in enumerate(zip(self._lines, self._axes)):
            for p,(line, slice_list) in enumerate(bllines):
                line.set_xdata(self._xdata)

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

    def put_message(self, vismsg):
        self._flags = vismsg.flag == 1 # shape=(nbaselines, nchannels)
        for bl in xrange(vismsg.nBaselines):
            self.put_cross(bl, vismsg.beamId, vismsg.polarisationId, vismsg.visibilities[bl, :])
            
        self.update_data()
        self.redraw()

class ZmqDataGenerator(object):
    def __init__(self, target):
        self.target = target
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.SUB)
        self.socket.connect(target)
        self.subscriptions = []
        self.subscribe(0, 'XX')
        self.init_mesg = VisibilityMessage(self.socket.recv_multipart())
        self.unsubscribe_all()

        print self.init_mesg
        m = self.init_mesg
        self.antenna1 = m.antenna1
        self.antenna2 = m.antenna2
        self.frequency = m.frequency
        self.nBaselines = m.nBaselines
        self.nChannels = m.nChannels
        self.chanWidth = m.chanWidth
        print "ANTENNAS"
        print self.antenna1
        print self.antenna2
        print "FREQ", len(self.frequency), self.chanWidth, self.nChannels
        print "Baseline", self.nBaselines


    def get_sockstring(self, beam, pol):
        assert pol.upper() in POL_STR
        assert beam >= 0
        s = '%d%s' % (beam, pol.upper())
        return s

    def subscribe(self, beam, pol):
        s = self.get_sockstring(beam, pol)
        assert s not in self.subscriptions
        self.socket.setsockopt_string(zmq.SUBSCRIBE, s.decode('ascii'))
        print "Subscribed", s
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
        vmsg = VisibilityMessage(msg)
        print "msg RX", msg[0], str(vmsg)

        t = Timer()
        t.start()
        nbl = vmsg.nBaselines # this includes autocorelations
        for p in plotters:
#            for bl in xrange(nbl):

#                p.put_cross(bl, vmsg.beamId, vmsg.polarisationId, vmsg.visibilities[bl, :])
            p.put_message(vmsg)
        t.stop()

#        t = Timer()
#        with t:
#            for p in plotters:
#                p.update_data()
#                p.redraw() # vis REDRAW complains with RuntimeError: main thread is not in main loop. No idea why.

#            plotters[0].redraw()


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

class VisibilityMessage(object):
    def __init__(self, msgs=None):
        if msgs is None:
            return

        self.pol_ident = msgs[0]
        r = BufferReader(msgs[1])
        self.timestamp = r.read_uint64()
        self.timestamp_dt = bat2utcDt(int(self.timestamp))
        self.beamId = r.read_uint32()
        self.polarisationId = r.read_uint32()
        self.nChannels = r.read_uint32()
        self.chanWidth = r.read_double()
        self.frequency = r.read_double(self.nChannels)/1e9
        self.nBaselines = r.read_uint32()
        self.antenna1 = r.read_uint32(self.nBaselines)
        self.antenna2 = r.read_uint32(self.nBaselines)
        self.visibilities = r.read_complex64(self.nBaselines*self.nChannels)
        self.visibilities.shape = (self.nBaselines, self.nChannels)
        self.flag = r.read_uint8(self.nBaselines*self.nChannels)
        self.flag.shape = (self.nBaselines, self.nChannels)

    def tofile(self, buff):
        self.timestamp.tofile(buff)
        self.beamId.tofile(buff)
        self.polarisationId.tofile(buff)
        self.nChannels.tofile(buff)
        self.chanWidth.tofile(buff)
        self.frequency.tofile(buff)
        self.nBaselines.tofile(buff)
        self.antenna1.tofile(buff)
        self.antenna2.tofile(buff)
        self.visibilities.flat.tofile(buff)
        self.flag.flat.tofile(buff)

    def tostring(self, s):
        s.write(self.timestamp.tostring())
        s.write(self.beamId.tostring())
        s.write(self.polarisationId.tostring())
        s.write(self.nChannels.tostring())
        s.write(self.chanWidth.tostring())
        s.write(self.frequency.tostring())
        s.write(self.nBaselines.tostring())
        s.write(self.antenna1.tostring())
        s.write(self.antenna2.tostring())
        s.write(self.visibilities.tostring())
        s.write(self.flag.tostring())

        

    def __str__(self):
        s = "t=%s beam=%d pol=%d chanWidth=%0.1f kHz Nbl: %d vishape: %s" % (
            self.timestamp_dt.isoformat(),
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
            try:
                gen.push(plotters)
            except:
                logging.exception("Error pusshing data")
#        print 'push took', str(t)


sel_regex = re.compile(r'sel\s*(.*)')
chan_regex = re.compile(r'(\d+)[\s+:](\d+)\s*')
scale_regex = re.compile(r'scale\s*((\d+\.?\d*)\s*(\d+\.?\d*))?')

class SubscriptionManager(object):
    def __init__(self, dgen, nant, npol, nbeams):
        self.ant_mask = np.ones(nant, dtype=np.bool)
        self.pol_mask = np.ones(npol, dtype=np.bool)
        self.beam_mask = np.zeros(nbeams, dtype=np.bool)
        nbl = nant*(nant+1)/2
        self.baseline_mask = np.zeros(nbl, dtype=np.bool)
        self.beam_mask[0] = True
        self.dgen = dgen
        self._calc_baseline_mask()
        self.include_autos = False
        self.include_cross = True

    def set_pol_mask(self, new_pol_mask):
        self.pol_mask[:] = new_pol_mask
        self.resubscribe()

    def set_ant_mask(self, new_ant_mask):
        self.ant_mask[:] = new_ant_mask[:]
        self._calc_baseline_mask()
        self.resubscribe()

    def set_beam_mask(self, new_beam_mask):
        self.beam_mask[:] = new_beam_mask[:]
        self.resubscribe()

    def set_baseline_mask(self, new_baseline_mask):
        self.baseline_mask[:] = new_baseline_mask[:]
        self.resubscribe()
    
    def _baseline_iter(self):
        for ibl, (a1, a2) in enumerate(zip(self.dgen.antenna1, self.dgen.antenna2)):
            yield ibl, a1, a2

    def _calc_baseline_mask(self):
        for ibl, a1, a2 in self._baseline_iter():
            if self.ant_mask[a1] == True or self.ant_mask[a2] == True:
                self.baseline_mask[ibl] = True

    def find_baseline_idx(self, a1f, a2f):
        af = (a1f, a2f) # tuple of antennas we're looking for
        afr = (a2f, a1f) # reversed version
        for ibl, a1, a2 in self._baseline_iter():
            a = (a1, a2)
            if a == af or a == afr: # if current = what we're looking for, or reverse
                return ibl

        raise ValueError('Unknown baseline combination: %s-%s' % (a1f, a2f))

    def resubscribe(self):
        self.dgen.unsubscribe_all()
#        print self.pol_mask
#        print self.beam_mask

        for ip in xrange(len(self.pol_mask)):
            for ib in xrange(len(self.beam_mask)):
                if self.pol_mask[ip] and self.beam_mask[ib]:
                    self.dgen.subscribe(ib, POL_STR[ip])
                    
    def mode_iter(self, mode):
        assert mode in PLOT_MODES
        if mode == 'a':
            for ibl in xrange(len(self.baseline_mask)):
                if not self.baseline_mask[ibl]:
                    continue

                a1 = self.dgen.antenna1[ibl]
                a2 = self.dgen.antenna2[ibl]
                label ='AK%d-AK%d' % (ANTENNA_LABELS[a1], 
                                      ANTENNA_LABELS[a2])
#                label = 'BL%d' % ibl

                if (self.include_autos and a1 == a2) or \
                        (self.include_cross and a1 != a2):
                    yield 0, ibl, label

        elif mode == 'b':
            for ibeam in xrange(len(self.beam_mask)):
                if not self.beam_mask[ibeam]:
                    continue

                label = 'B%d'%ibeam

                yield 1, ibeam, label

        elif mode == 'p':
            for ip in xrange(len(self.pol_mask)):
                if not self.pol_mask[ip]:
                    continue

                label = POL_STR[ip]

                yield 2, ip, label


def str2slice(s):
    parts = map(int, s.strip().split(':'))
    if len(parts) == 1:
        sl = slice(parts[0], parts[0]+1)
    else:
        sl = slice(*parts)

    print sl, parts
    return sl

def parse_command(cmd, plt, gen):
    import Tkinter
    bits = cmd.split()
    scalem = scale_regex.match(cmd)
    selm = sel_regex.match(cmd)


    if cmd == 'quit' or cmd == 'exit':
        print 'Quitting'
        Tkinter.tkinter.quit()
        #sys.exit(0)
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
    elif cmd == 'dba':
        plt.set_variable('dba')
    elif cmd == 'replot':
        plt.replot()
    elif cmd == 'x':
        plt.toggle_xmode()
    elif cmd.startswith('panel'):
        panval = cmd.split()[1]
        plt.set_panel(panval)
    elif cmd.startswith('acs'):
        plt._mgr.include_autos = True
        plt.replot()
    elif cmd.startswith('noacs'):
        plt._mgr.include_autos = False
        plt.replot()
    elif cmd.startswith('ccs'):
        plt._mgr.include_cross = True
        plt.replot()
    elif cmd.startswith('noccs'):
        plt._mgr.include_cross = False
        plt.replot()
    elif cmd.startswith('stack'):
        stackval = cmd.split()[1]
        plt.set_stack(stackval)
    elif selm is not None:
        pol_mask = plt._mgr.pol_mask*False
        for sel_str in selm.group(1).split():
            sel_str = sel_str.upper()
            if sel_str in POL_STR:
                pol_mask[POL_STR.index(sel_str)] = True
                         
        assert sum(pol_mask) >= 1, 'Not polarisation selected %s' % pol_mask
                
        plt._mgr.set_pol_mask(pol_mask)
        plt.replot()
        
    elif cmd.startswith('ibl'):
        bl_mask = plt._mgr.baseline_mask*False
        if len(bits) == 1:
            bl_mask[:] = True
        else:
            for bl_str in bits[1:]:
                if len(bl_str) == 2:
                    a1, a2 = map(int, bl_str)
                elif '-' in bl_str:
                    a1, a2 = map(int, bl_str.split('-'))
                else:
                    raise ValueError('Invalid baseline spec: %s' % bl_str)
                
                blidx = plt._mgr.find_baseline_idx(a1, a2)
                bl_mask[blidx] = True

        plt._mgr.set_baseline_mask(bl_mask)
        plt.replot()

    elif cmd.startswith('iarray'):
        arr_mask = plt._mgr.ant_mask*False
        if len(bits) == 1:
            arr_mask[:] = True
        else:
            for a in bits[1:]:
                arr_slice = str2slice(a)
                arr_mask[arr_slice] = True
                
        print "Array MASK", arr_mask
        print "Antenna labels", ['AK%d' %ak for ak in ANTENNA_LABELS]
        plt._mgr.set_ant_mask(arr_mask)
        
        plt.replot()

    elif cmd.startswith('beam'):
        if len(bits) >= 1:
            beam_mask = plt._mgr.beam_mask*False
            print "BMASK BEFORE"
            for b in bits[1:]:
                beam_slice = str2slice(b)
                beam_mask[beam_slice] = True
                print b, beam_slice

            print 'BMASK', beam_mask
            plt._mgr.set_beam_mask(beam_mask)
            plt.replot()


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

    elif cmd.startswith('flag'):
        plt.set_flag(True)
    elif cmd.startswith('noflag'):
        plt.set_flag(False)
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
    elif cmd.startswith('layout') or cmd.startswith('nxy'):
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
            

    elif cmd.startswith('legon'):
        plt.set_legend(True)
    elif cmd.startswith('legoff'):
        plt.set_legend(False)
    elif cmd.startswith('gridon'):
        plt.set_grid(True)
    elif cmd.startswith('gridoff'):
        plt.set_grid(False)
    elif cmd.startswith('?') or cmd.startswith('help'):
        print_help()
    else:
        print 'Unknown command: %s' % cmd

def print_help():
    s = """SPD for ASKAP (C) CSIRO 2014
--- Variable selection ---
a - amplitudes
dba - amplitudes (dB)
p - phases
r - real
i - imaginary
d - difference between current and saved = abs(curr - saved)
q - quotition of current and saved = abs(curr)/abs(saved)

--- Data selection ---
sel xx|yy|xy|yx - select stokes XX YY YX XY (can do multiple separated by space)
beam X[ Y[ Z[ ..]]] - select beam s (e.g. 'beam 0 1 2 3 4' or for a range 'beam start:stop:step' e.g. 'beam 0:6:2')
iarray A B C D - Select array (by indices) e.g. 'iarray 0 1 2 3'. For a range, do i'array 0:6'. For all, just do iarray with no arguments.
ibl AB[ CD[ EF[ ..]] - select baseline (by indicies). E.g 'ibl 01 02 03'. One day, for ASKAP we'll have > 9 antennas, so you can also separate indices by a hyphen. e.g. 'ibl 0-1 0-2 0-35'. Type 'ibl' to get all baselines (same as 'iarray')
flag - mask out flagged data
noflag - Do not mask out flagged data

-- Plot manipulation --
scale [start end] - set y scale between start and end. If not specified, autoscale
chan [start end] - plot channels from start to end. If not specified, autoscale
stack a|b|p - Set multiple lines on each panel to be (a)ntennas, (b)eams or (p)olarisations
panel a|b|p - Set each panel in sequence to be (a)ntennas, (b)eams or (p)olarisations
x - Swap between frequency (GHz) and channel number
acs - Plot autocorrelation
noacs - don't plot autocorrelation
ccs - Plot cross correlation
noccs - Don't plot cross correlations

--- Previous buffer ---
save - Save current data to previous buffer, so you can plot with 'd' or 'q'
rsave - Save new data to previous buffer, so you can plot with 'd' or 'q'
norsave - Stop saving new data to previous buffer

--- Averaging ---
avg - Start averaging in time (see pstat for number of averages)
noavg - Stop averaging in time

--- Miscellaneous ---
legon - Plot legend
legoff - Turn off legend
gridon - Draw a grid on the plots
gridoff - Turn off grid
pstat - print plotting statistics to stdout
write FILE - Save figure to file: Supported formats from extension (e.g. spec.png, spec.pdf)
layout X Y - Make figure X subfigs wide and Y subfigs tall
nxy - same as 'layout'
replot - Replot the figure.
help - print this out
quit - Leave program
exit - Same as quit
? - print this out

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
    parser.add_argument(dest='infiles', nargs=1, help='ZMQ url for vis service. E..g. tcp://aktos01:9002')
    parser.set_defaults(verbose=False)
    values = parser.parse_args()
    if values.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)

#    nant = 6;     nbeams = 1;     nchan = 16416
    #nant = 6; nbeams = 9 ; nchan = 16384
#    nchan = 304
    nbeams= 9
#    nchan = 16416
    npol = 4
    gen = ZmqDataGenerator(values.infiles[0])
    nant = len(set(gen.antenna1))
    freq = gen.frequency

    plt = SpectrumPlotter(gen, nant, nbeams, npol, freq)
#    vis = SummaryPlotter(nant, nbeams, npol, nchan, start_freq, chanbw)
    vis = None

    
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
