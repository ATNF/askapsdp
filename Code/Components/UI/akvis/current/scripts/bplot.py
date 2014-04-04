#!/usr/bin/env python
"""
A class that plots visibility spetra like SPD does at the ATCA


AAARGH:
For some reason 2 plot windows doens't work - TK bug no doubt.

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
    parser.add_argument('-c', '--channel', dest='channel', help='Channels to plot. Leave empty for all or start[:stop[:step]]', default=None)
    
    parser.set_defaults(verbose=False)
    values = parser.parse_args()
    if values.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)


    ant_slice = str2slice(values.antenna)
    beam_slice = str2slice(values.beam)
    chan_slice = str2slice(values.channel)


    for f in values.infiles:
        s = Source(f, values.type)
        print f, 'most recent soln', s.most_recent_solution_id
        ac = s.most_recent_accessor
        bp_g1, bp_g2 = ac.bandpass[ant_slice, beam_slice, chan_slice]
        
        nant, nbeam, nchan = bp_g1.shape
        
        figure()
        title(f + ' bandpass')
        subplot(2,2,1)
        for b in xrange(nbeam):
            plot(abs(bp_g1[:, b, :]).T)
            ylabel('Amplitude')
            title('G1')
            subplot(2,2,3)
            plot(degrees(angle(bp_g1[:, b, :]).T))
            ylabel('Phase (deg)')
            xlabel('channel')
            subplot(2,2,2)
            plot(abs(bp_g2[:, b, :]).T)
            title('G2')
            subplot(2,2,4)
            plot(degrees(angle(bp_g2[:, b, :]).T))
            xlabel('channel')


        
    show()

if __name__ == '__main__':
    _main()
