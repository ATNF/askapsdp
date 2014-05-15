#!/usr/bin/env python
"""
Summarises cross correlations

Copyright (C) Keith Bannister 2014
"""
import pylab
import numpy as np
import os
import sys
import logging
from  scipy.optimize import curve_fit


def vis_func_real(freq, amp, delay, phase):
#    print 'VIS FUNC', freq.shape, amp.shape, delay.shape, phase.shape
        
    t = 2*np.pi*freq*delay + phase
    n = len(freq)/2
    v = np.empty_like(freq)
    v[0:n] = amp * np.cos(t[0:n])
    v[n:] = amp * np.sin(t[n:])

    return v

def vis_func(freq, amp, delay, phase):
    v = amp * np.exp(1j*(2*np.pi*freq*delay + phase))
    return v


def fit_apd(spec, freq):
    """fits a spectrum for amplitude, phase and delay"""
    f2 = np.hstack((freq, freq))
    spec2 = np.hstack((np.real(spec), np.imag(spec)))
    a0 = np.mean(abs(spec))
    phase = np.mean(np.angle(spec))
    delay = 0

    p0 = [a0, delay, phase]
    #f = curve_fit(vis_func, xdata=f2, ydata=spec2, p0=None)
    f = curve_fit(vis_func_real, xdata=f2, ydata=spec2, p0=None)

    return f


def fit_apd_basic(spec, freq):
    """ Apparently this is how CABB does it
    Which worries me somewhat.
    """
    amp = np.mean(abs(spec))
    ang = np.unwrap(np.angle(spec))
    phase = np.mean(ang)
    pfit = np.polyfit(freq, ang, 1)
    B = freq[-1] - freq[0]
    delay = -pfit[1] * B/2.
    return (amp, delay, phase), None
    
class Summariser(object):
    def __init__(self, nant, nbeams, npol, nchan, fstart, chanbw):
        self._nant = nant
        self._nbeams = nbeams
        self._npol = npol
        self._nchan = nchan
        self._tvchan = (0, nchan-1)
        self._delavg = 1
        self._freqs = np.arange(nchan)*chanbw + fstart

    def get_apd(self, spec):
        popt, pcov = fit_apd(spec, self._freqs)
        return popt, pcov


def _main():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Script description')
    parser.add_argument('-v', '--verbose', dest='verbose', action='store_true', help='Be verbose [Default %default]')
    parser.set_defaults(verbose=False)
    values = parser.parse_args()
    if values.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)
    
    amp = 1.1
    delay = 1.3
    phase = 0.7
    nchan = 301
    freq = np.arange(304)*1.0e-3 + 1.4
    f2 = np.hstack((freq, freq))
    spec = vis_func(freq, amp, delay, phase)
#    spec = amp * np.exp(1j*(2*np.pi*freq*delay + phase))


    popt, pcov = fit_apd(spec, freq)
    poptb, pcovb = fit_apd_basic(spec, freq)


    fspec = vis_func(freq, *popt)
    fspecb = vis_func(freq, *poptb)

    plot_spec(freq, spec)
    plot_spec(freq, fspec,'x')
    plot_spec(freq, fspecb,'+')
    print popt, pcov
    amp_f, delay_f, phase_f = popt
    amp_fb, delay_fb, phase_fb = poptb
    print "amp", amp, amp_f, amp_fb
    print "phase", phase, phase_f, phase_fb
    print "delay", delay, delay_f, delay_fb
    pylab.show()



def plot_spec(freq, spec, *args, **kwargs):
    pylab.subplot(3,1,1)
    pylab.plot(freq, np.abs(spec), *args, **kwargs)
    pylab.ylabel('amp')
    pylab.subplot(3,1,2)
    pylab.plot(freq, np.angle(spec), *args, **kwargs)
    pylab.ylabel('phase')

    pylab.subplot(3,1,3)
    pylab.plot(freq, np.real(spec), *args, **kwargs)
    pylab.plot(freq, np.imag(spec), *args, **kwargs)
    pylab.ylabel('real/imag')


    
    

if __name__ == '__main__':
    _main()
