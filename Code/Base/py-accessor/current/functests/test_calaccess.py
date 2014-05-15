#!/usr/bin/python
#
# Copyright (c) 2010 CSIRO
# Australia Telescope National Facility (ATNF)
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# PO Box 76, Epping NSW 1710, Australia
# atnf-enquiries@csiro.au
#
# This file is part of the ASKAP software distribution.
#
# The ASKAP software distribution is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the License
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA.
#
from askap.accessors._accessors import CalSourceWrap
from askap.accessors.calibration import Source
from askap.accessors.calibration import Accessor
import numpy as np

from unittest import TestCase


# This test was written for python 2.7, fix up 2.6
if not hasattr(TestCase, "assertIsInstance"):
    def assertIsInstance(_self, obj, clstyp):
        _self.assertTrue(isinstance(obj, clstyp))
    def assertIsNotNone(_self, obj):
        _self.assertTrue(obj is not None)

    TestCase.assertIsInstance = assertIsInstance 
    TestCase.assertIsNotNone = assertIsNotNone

def mod_file(fin):
    import os
    """ Returns a path to a file in the same directory as this module"""
    return os.path.join(os.path.dirname(__file__), fin)

# def test_calaccess_wrapper():
#     parset = """
#     calibaccess = 'table'
#     calibaccess.table = 'calibdata.tab'
#     """

#     w = CalSourceWrap(parset)
#     print 'Most recent solution', w.mostRecentSolution()
# #    print 'SolutionID', w.solutionID()
#     soln = w.roSolution(w.mostRecentSolution())
#     print soln.gain(0, 0).g1
#     print soln.jones(0, 0, 0)
#     bp = soln.bandpass(0, 0, 0)
#     print 'Bandpass', bp.g1, bp.g2
#     print type(soln.jones(0, 0, 0))
# #    print soln.jones(54, 0, 0)


class TestCalSource(TestCase):
    def setUp(self):
        # this dataset has
        # 36 antennas, 1 beam and 8 channels in the bandpass table
        
        self.ca = Source(mod_file('calibdata.tab'))
        self.assertEquals(self.ca.most_recent_solution_id, 0)
        ac = self.ca.most_recent_accessor
        self.assertIsNotNone(ac)
        self.ac = ac
        self.ant, self.beam, self.chan = 0, 0, 0


    def test_table_access(self):
        src = Source(mod_file('calibdata.tab')).most_recent_accessor
        self.assertNotEquals(tuple(src.bandpass[:,:,:][0].shape), (0, 0, 0))
        self.assertNotEquals(tuple(src.gain[:,:][0].shape), (0, 0))
        self.assertNotEquals(tuple(src.leakage[:,:][0].shape), (0, 0))


    def test_table2_access(self):
        src = Source(mod_file('caldata1.tab')).most_recent_accessor
#        self.assertNotEquals(src.bandpass[:,:,:], (0, 0, 0))
        self.assertNotEquals(tuple(src.gain[:,:][0].shape), (0, 0))
        self.assertNotEquals(tuple(src.leakage[:,:][0].shape), (0, 0))

    def test_parset_access(self):
        src = Source(mod_file('rndgains.in'), type='parset').most_recent_accessor
#        self.assertNotEquals(src.bandpass[:,:,:], (0, 0, 0))
        self.assertNotEquals(tuple(src.gain[:,:][0].shape), (0, 0))
        self.assertNotEquals(tuple(src.leakage[:,:][0].shape), (0, 0))

    def test_gain(self):
        gain = self.ac.get_gain(self.ant, self.beam)
        self.assertIsInstance(gain.g1, complex)
        self.assertIsInstance(gain.g2, complex)
        self.assertIsInstance(gain.g1IsValid, bool)
        self.assertIsInstance(gain.g2IsValid, bool)

    def test_leakage(self):
        leakage = self.ac.get_leakage(self.ant, self.beam)
        self.assertIsInstance(leakage.d12, complex)
        self.assertIsInstance(leakage.d21, complex)
        self.assertIsInstance(leakage.d12IsValid, bool)
        self.assertIsInstance(leakage.d21IsValid, bool)

    def test_bandpass(self):
        bp = self.ac.get_bandpass(self.ant, self.beam, self.chan)
        self.assertIsInstance(bp.g1, complex)
        self.assertIsInstance(bp.g2, complex)
        self.assertIsInstance(bp.g1IsValid, bool)
        self.assertIsInstance(bp.g2IsValid, bool)

    def test_bandpass_crazy(self):
        self.assertRaises(RuntimeError, self.ac.get_bandpass, 36+1, 0, 0)
        self.assertRaises(RuntimeError, self.ac.get_bandpass, 0, 0+1, 0)
        self.assertRaises(RuntimeError, self.ac.get_bandpass, 0, 0, 8+1)

    def test_jobes_crazy(self):
        self.assertRaises(RuntimeError, self.ac.get_jones, 36+1, 0, 0)
        self.assertRaises(RuntimeError, self.ac.get_jones, 0, 0+1, 0)
        self.assertRaises(RuntimeError, self.ac.get_jones, 0, 0, 8+1)

    def test_jones(self):
        jones = self.ac.get_jones(self.ant, self.beam, self.chan)
        self.assertEquals(jones.shape, (2,2))

    def test_bandpass_slice_all(self):
        g1, g2 = self.ac.bandpass[:, :, :]
        self.assertEquals(g1.shape, (36, 1, 8))
        self.assertEquals(g2.shape, (36, 1, 8))
        self.assertEquals(g1.dtype, np.complex)
        self.assertEquals(g2.dtype, np.complex)

    def assertNpEquals(self, a1, a2, msg=''):
        self.assertEquals(a1.shape, a2.shape)

        are_equal = np.all(a1 == a2)
        if not are_equal:
            first_error = np.where(a1 != a2)[0]
            self.assertTrue(are_equal, "Arrays not equal. First bad a1[{0}]: {1} != a2[{0}]: {2}".format(first_error, a1[first_error], a2[first_error]))

    def test_bandpass_slice_oneant(self):
        g1, g2 = self.ac.bandpass[:, :, :]
        g1a1, g2a1 = self.ac.bandpass[0, :, :]
        self.assertEquals(g1a1.shape, (1, 1, 8))
        self.assertEquals(g2a1.shape, (1, 1, 8))
        self.assertNpEquals(g1a1[0, :, :], g1[0, :, :])
        self.assertNpEquals(g2a1[0, :, :], g2[0, :, :])

    def test_bandpass_slice_antslice(self):
        g1, g2 = self.ac.bandpass[:, :, :]

        aslice = slice(2, 19, 3)
        g1a1, g2a1 = self.ac.bandpass[aslice, :, :]
        self.assertEquals(g1a1.shape, g2a1.shape)
        self.assertNpEquals(g1a1[:, :, :], g1[aslice, :, :])
        self.assertNpEquals(g2a1[:, :, :], g2[aslice, :, :])

    def test_bandpass_slice_onebeam(self):
        g1, g2 = self.ac.bandpass[:, 0, :]
        self.assertEqual(g1.shape, (36, 1, 8))
        self.assertEqual(g2.shape, (36, 1, 8))

    def test_bandpass_slice_onechan(self):
        g1, g2 = self.ac.bandpass[:, :, 1]
        self.assertEqual(g1.shape, (36, 1, 1))
        self.assertEqual(g2.shape, (36, 1, 1))
        g1all, g2all = self.ac.bandpass[:, :, :]
        self.assertNpEquals(g1[:, :, 0], g1all[:, :, 1])
        self.assertNpEquals(g2[:, :, 0], g2all[:, :, 1])

    def test_gain_crazy(self):
        self.assertRaises(RuntimeError, self.ac.get_gain, 129,129)

    def test_gain_slice_all(self):
        g1, g2 = self.ac.gain[:, :]

        # Max's example looks like it has 128 antennas and 128 beams?
        self.assertEquals(g1.shape, (128, 128))
        self.assertEquals(g2.shape, (128, 128))
        self.assertEquals(g1.dtype, np.complex)
        self.assertEquals(g2.dtype, np.complex)
        self.assertTrue(np.all(g1.mask == True))

    def test_leakage_slice_all(self):
        g1, g2 = self.ac.leakage[:, :]

        # Max's example looks like it has 128 antennas and 128 beams?
        self.assertEquals(g1.shape, (128, 128))
        self.assertEquals(g2.shape, (128, 128))
        self.assertEquals(g1.dtype, np.complex)
        self.assertEquals(g2.dtype, np.complex)
        self.assertTrue(np.all(g1.mask == True))

    def test_jones_slice_all(self):
        jones = self.ac.jones[:, :, :]
        self.assertEquals(jones.shape, (36, 1, 8, 2, 2))
        self.assertEquals(jones.dtype, np.complex)
        self.assertTrue(np.all(jones.mask == True))

