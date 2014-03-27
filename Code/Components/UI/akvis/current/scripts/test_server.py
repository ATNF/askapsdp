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
from askap.akvis.testutils import Timer
from askap.time import bat2utcDt, utcDt2bat
import numpy as np
import threading
import time
from askap.akvis import corr_summary
import zmq
import struct
from spd import POL_STR, VisibilityMessage
import cStringIO as StringIO
import datetime
import pytz

def single(v, dtype):
    return np.array([v], dtype=dtype)

def complex_noise(amp, n):
    noise = np.random.randn(n)*amp + 1j*np.random.randn(n)*amp
    return noise

class TestDataGenerator(object):
    def __init__(self, antenna1, antenna2, freq, nbeams):
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.PUB)
        self.socket.bind("tcp://*:9002")
        self.antenna1 = np.array(antenna1, np.uint32)
        self.antenna2 = np.array(antenna2, np.uint32)
        self.freq = np.array(freq, np.double)
        self.nbeams = nbeams
        self._simstep = 0
        nant = len(set(self.antenna1))
        self.nbl = nant * (nant + 1)/2
        self.nchan = len(self.freq)
        self.flag = np.zeros((self.nbl, self.nchan), dtype=np.uint8)
        self.flag[2, 0::10] = 1
        self.chanWidth = single(self.freq[1] - self.freq[0], np.double)
        self._noise_amp = 0.0

    def _gen_noise(self):
        return complex_noise(self._noise_amp, self.nchan)

    def push(self, delay_step=0.01):
        t = Timer()
        t.start()
        nant = len(self.antenna1)
        channels = np.arange(self.nchan)/float(self.nchan)
        for beam in xrange(self.nbeams):
            for ipol in xrange(4):
                visibilities = np.ones((self.nbl, self.nchan), dtype=np.complex64)
                for ibl, (a1, a2) in enumerate(zip(self.antenna1, self.antenna2)):
                    delay = np.sin(self._simstep*delay_step*float(ibl)/float(self.nbl))
                    phase = 0.1
                    phases = 2*np.pi*1j*(channels*delay + phase)
                    if a1 == a2:
                        phases *= 0

                    ispec = np.exp(phases)
                    v = self._gen_noise()
                    if ipol in (0, 1):
                        v =+ ispec

                    visibilities[ibl, :] = ispec

                self.put_message(beam, ipol, visibilities)
                
        
        self._simstep += 1.
        time.sleep(5)

    def make_message(self, beam, polid, visibilities):
        v = VisibilityMessage(None)
        dt = datetime.datetime.now(pytz.utc)
        v.timestamp = single(utcDt2bat(dt), np.uint64)
        v.frequency = self.freq
        v.nChannels= single(len(self.freq), np.int32)
        v.antenna1 = self.antenna1
        v.antenna2 = self.antenna2
        v.nBaselines = single(len(self.antenna1), np.int32)
        v.chanWidth = single(self.chanWidth, np.double)
        v.flag = self.flag
        v.beamId = single(beam, np.uint32)
        v.polarisationId = single(polid, np.uint32)
        v.visibilities = visibilities
        return v

    def check_messages(self, m1, m2):
        assert m1.nBaselines == len(self.antenna1)
        assert np.all(m1.frequency == self.freq)
        assert np.all(m1.flag == self.flag)
        assert np.all(m1.antenna1 == self.antenna1)
        assert np.all(m1.antenna2 == self.antenna2)

        assert m1.timestamp == m2.timestamp
        assert m1.nBaselines == m2.nBaselines
        assert m1.chanWidth == m2.chanWidth
        assert m1.beamId == m2.beamId
        assert m1.polarisationId == m2.polarisationId
        assert np.all(m1.antenna1 == m2.antenna1)
        assert np.all(m1.antenna2 == m2.antenna2)
        assert m1.flag.shape == m2.flag.shape, '%s %s' % (m1.flag.shape, m2.flag.shape)
        assert np.all(m1.flag == m2.flag)
        assert np.all(m1.visibilities == m2.visibilities)
        
    def put_message(self, beam, polid, visibilities):
        v = self.make_message(beam, polid, visibilities)
        sio = StringIO.StringIO()
        v.tostring(sio)
        s = sio.getvalue()
        visname = '%d%s' % (beam, POL_STR[polid])
        msg = [visname, s]
        v2 = VisibilityMessage(msg)
        self.check_messages(v, v2)
        print "Sending message", visname, v.timestamp, v.visibilities[:,0:10]
        self.socket.send_multipart(msg)
        print "Sent"

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
    nbeams= 9
    nchan = 304
    npol = 4
    ant1 = [0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 5]
    ant2 = [0, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 2, 3, 4, 5, 3, 4, 5, 4, 5, 5]


    start_freq = 700e6# Hz
    chanbw = 1e6 # Hz
    freq = np.arange(nchan) * chanbw + start_freq
    gen = TestDataGenerator(ant1, ant2, freq, nbeams)
    while True:
        try:
            gen.push()
        except KeyboardInterrupt:
            break
        except AssertionError:
            raise
        except:
            logging.exception("Couldnt push")


if __name__ == '__main__':
    _main()
