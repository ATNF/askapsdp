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

def _main():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Plots bandpass')
    parser.add_argument('-v', '--verbose', dest='verbose', action='store_true', help='Be verbose [Default %default]')
    parser.add_argument(dest='infiles', nargs='*', help='Data files to look at')
    parser.add_argument('-t', '--type', dest='type', help='Type of files: table|parset|raw', default='table')
    parser.add_argument('-a', '--antenna', dest='antenna', type=int, help='Antena number to plot', default=0)
    parser.add_argument('-b', '--beam', dest='beam', type=int, help='Beam numebr to plot', default=0)
    
    parser.set_defaults(verbose=False)
    values = parser.parse_args()
    if values.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)

    for f in values.infiles:
        s = Source(f, values.type)
        print f, 'most recent soln', s.most_recent_solution_id
        ac = s.most_recent_accessor
        bp_g1, bp_g2 = ac.bandpass[values.antenna, values.beam, :]
        
        figure()
        title(f + ' bandpass')
        subplot(2,2,1)
        plot(abs(bp_g1[:, 0, :]).T)
        ylabel('Amplitude')
        title('G1')
        subplot(2,2,3)
        plot(degrees(angle(bp_g1[:, 0, :]).T))
        ylabel('Phase (deg)')
        xlabel('channel')
        subplot(2,2,2)
        plot(abs(bp_g2[:, 0, :]).T)
        title('G2')
        subplot(2,2,4)
        plot(degrees(angle(bp_g2[:, 0, :]).T))
        xlabel('channel')


        
    show()

if __name__ == '__main__':
    _main()
