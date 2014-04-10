#!/usr/bin/env python
"""
A class that plots gain solutions

Copyright (C) Keith Bannister 2014
"""
import os
import sys
import logging
import threading
import time
from askap.accessors.calibration import Source
from pylab import *
from spd import str2slice

def _main():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Plots bandpass')
    parser.add_argument('-v', '--verbose', dest='verbose', action='store_true', help='Be verbose')
    parser.add_argument(dest='infiles', nargs='*', help='Data files to look at')
    parser.add_argument('-t', '--type', dest='type', help='Type of files: table|parset|raw', default='table')
    parser.add_argument('-a', '--antenna', dest='antenna', help='Antena number to plot. Leave empty for all or start[:stop:[step]]', default=None)
    parser.add_argument('-b', '--beam', dest='beam', help='Beam number to plot. Leave empty for all or start[:stop[:step]]', default=None)
    parser.add_argument('-s', '--solnid', dest='solnid', help='Solution id to plot. Leave empty for last', default=None)
    
    parser.set_defaults(verbose=False)
    values = parser.parse_args()
    if values.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)


    ant_slice = str2slice(values.antenna)
    beam_slice = str2slice(values.beam)

    for f in values.infiles:
        s = Source(f, values.type)
        ntime = s.most_recent_solution_id
        print f, 'most recent soln ID', s.most_recent_solution_id
        
        data = None
        for t in xrange(ntime):
            ac = s.get_accessor_for_id(t)
            g1, g2 = ac.gain[ant_slice, beam_slice]
            if data is None:
                nant, nbeam = g1.shape
                data = np.zeros((nant, nbeam, ntime, 2), dtype=np.complex)

            data[:,:,t,0] = g1
            data[:,:,t,1] = g2
        
        
        figure()
        title(f + ' gains')
        for b in xrange(nbeam):
            subplot(2,2,1)
            plot(abs(data[:, b, :, 0].T))
            ylabel('Amplitude')
            title('G1')
            subplot(2,2,3)
            plot(degrees(angle(data[:,b,:,0].T)))
            ylabel('Phase (deg)')
            xlabel('Time step')
            subplot(2,2,2)
            plot(abs(data[:,b,:,0].T))
            title('G2')
            subplot(2,2,4)
            plot(degrees(angle(data[:,b,:,0].T)))
            xlabel('Time step')


        
    show()

if __name__ == '__main__':
    _main()
