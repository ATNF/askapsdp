# Python Wrapper for CalibAccessFactory
#
# @copyright (c) 2013 CSIRO
# Australia Telescope National Facility (ATNF)
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# PO Box 76, Epping NSW 1710, Australia
# atnf-enquiries@csiro.au
#
# This file is part of the ASKAP software distribution.
#
# The ASKAP software distribution is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# @author Keith Bannister <keith.bannister@csiro.au>
#

import os
import sys
from askap.accessors._accessors import CalSourceWrap
import numpy as np
import numpy.ma as ma
from itertools import islice

class Source(object):
    """Represents a calibration data source.

    The Source an be backed by a range of different storage methods. The storage
    method is chosen by the arguments to the contructor.

    if type = 'table', f is interpreted as a path to a Calibration table, 
    in measurement set format.
    
    if type = 'parset', f is interpreted as a path to a Parset table
    
    if type = 'raw', f is passed directoy to CalibAccessFactory.cc,
    after being converted to a parset. This gives full control of the factory
    
    :param f: path to table, parset or string containing parset configuration
    :param type: The way to interpret f. Either 'table', 'parset', or 'raw'
    """
    def __init__(self, f, type='table'):
        """Creates a Calibration Source"""
        if type == 'table':
            parset = """
 calibaccess = 'table'
 calibaccess.table = '{0}'
 """.format(f)
        elif type == 'parset':
            parset = """
 calibaccess = 'parset'
 calibaccess.parset = '{0}'
 """.format(f)
        elif type == 'raw':
            parset = f

        self.__source = CalSourceWrap(parset)

    @property
    def most_recent_accessor(self):
        """Contains the most recent Accessor
        Equivalent to self.get_accessor_for_id(self.most_recent_solution_id)
        """
        return self.get_accessor_for_id(self.most_recent_solution_id)

    def get_accessor_for_id(self, id):
        """Returns an accessor for the given solution id
        
        :param id: an integer specifying the id
        """
        return Accessor(self.__source.roSolution(id))

    @property
    def most_recent_solution_id(self):
        """The solution id of the most recent solution in this Source"""
        return self.__source.mostRecentSolution()

    def get_solution_id_for_time(self, time):
        """Get a solution ID for a given time
        
        :param time: double precision. Time stamp in secodns since MJD 0
        """
        # TODO: Check time is a double
        return self.__source.solutionID(time)

def _slice2xrange(s):
    """
    >>> for i in _slice2xrange(slice(0, 3, None)): print i
    0
    1
    2
    >>> for i in _slice2xrange(3): print i
    3
    """
    if isinstance(s, slice):
        it = iter(xrange(s.start or 0, s.stop or sys.maxint, s.step or 1))
    else:
        it = iter(xrange(s, s+1, 1))
    return it

def iter_until_error(func, it):
    for i in it:
        try:
            v = func(i)
            yield v
        except RuntimeError, e: # Thrown when we hit the end of the accessor
            # TODO: Check the runtime error message is the one when we hit the end,
            # Not another one
            break


def _func_iterator3(func, itemslice):
    """Iterates over 3 dimensions and returns a 3 D set of lists
    """
    assert len(itemslice) == 3, "Must supply a 3D slice"
    aslice, bslice, cslice = itemslice

    adata = []
    for ant in _slice2xrange(aslice):
        bdata = []
        for beam in _slice2xrange(bslice):
            f = lambda chan: func(ant, beam, chan)
            cdata = list(iter_until_error(f, _slice2xrange(cslice)))
            if cdata:
                bdata.append(cdata)
            else: # this antenna and beam are invalid. 
                break
        
        if bdata:
            adata.append(bdata)
        else: # no more antennas
            break

    return adata

def _func_iterator2(func, itemslice):
    """Iterates over 2 dimensions and returns a 2 D set of lists
    """
    assert len(itemslice) == 2, "Must supply a 2D slice"
    aslice, bslice = itemslice
    adata = []
    for ant in _slice2xrange(aslice):
        f = lambda beam: func(ant, beam)
        bdata = list(iter_until_error(f, _slice2xrange(bslice)))
        if bdata:
            adata.append(bdata)
        else: # this antenna is bad
            break

    return adata
    
class _BandpassSlicer(object):
    """Allows sliced view of the bandpass solution (see get_bandpass)

    3 slices should be supplied for [antenna, beam, channel]

    Returns tuple (g1, g2) where g1 and g2 are  3D masked numpy arrays
    [nant, nbeam, nchan], for the g1 and g2 terms.
    
    The masks are set as valid if the corresponding IsValid function in the 
    JonesDTerm returns True.

    """
    def __init__(self, func):
        self.func = func

    def __getitem__(self, item):
        g1term = lambda ant, beam, chan: self.func(ant, beam, chan).g1
        g2term = lambda ant, beam, chan: self.func(ant, beam, chan).g2
        g1valid = lambda ant, beam, chan: not self.func(ant, beam, chan).g1IsValid
        g2valid = lambda ant, beam, chan: not self.func(ant, beam, chan).g2IsValid
        
        g1_cube = _func_iterator3(g1term, item)
        g2_cube = _func_iterator3(g2term, item)
        g1valid_cube = _func_iterator3(g1valid, item)
        g2valid_cube = _func_iterator3(g2valid, item)

        g1 = ma.masked_array(g1_cube, mask=g1valid_cube)
        g2 = ma.masked_array(g2_cube, mask=g2valid_cube)
        return g1, g2
        

class _GainSlicer(object):
    """Alows sliced view of the gain parameters (see get_gain)
    
    2 Slices should be supplied for [antenna, beam]

    Returns a tuple containing (g1, g2) where g1 and g2 are numpy masked
    arrays contaiging the values at the specified slices.
    
    The masks are set as valid if the corresponding IsValid function in the 
    JonesDTerm returns True.
    """
    def __init__(self, func):
        self.func = func

    def __getitem__(self, item):
        g1term = lambda ant, beam: self.func(ant, beam).g1
        g2term = lambda ant, beam: self.func(ant, beam).g2
        g1valid = lambda ant, beam: not self.func(ant, beam).g1IsValid
        g2valid = lambda ant, beam: not self.func(ant, beam).g2IsValid
        
        g1_cube = _func_iterator2(g1term, item)
        g2_cube = _func_iterator2(g2term, item)
        g1valid_cube = _func_iterator2(g1valid, item)
        g2valid_cube = _func_iterator2(g2valid, item)

        g1 = ma.masked_array(g1_cube, mask=g1valid_cube)
        g2 = ma.masked_array(g2_cube, mask=g2valid_cube)
        return g1, g2


class _JonesSlicer(object):
    """Allows sliced view of the full Jones matrix (see get_jones)

    3 slices should be supplied for [antenna, beam, channel]

    Returns a 5D masked numpy array: [nant, nbeam, nchan, 2, 2] where the final 2
    dimensions are the Jones matrix for the given antenna and beam.
    
    The mask for all Jones terms are set as valid if jonesValid() returns true
    for the given antenna, beam and channel.
    """
    def __init__(self, func, maskfunc):
        self.func = func
        
        # Convert a single boolean output to mask out all the entries in the 
        # 2x2 jones matrix
        mask22 = np.array([[True, True], [True, True]])
        self.maskfunc = lambda ant, beam, chan: (not maskfunc(ant, beam, chan))*mask22

    def __getitem__(self, item):
        # jones_data has shape is Nant x Nbeam x Nchan x 2 x 2
        jones_data = _func_iterator3(self.func, item) 
        
        # jones_mask has shape is Nant x Nbeam x Nchan x 2 x 2
        jones_mask = _func_iterator3(self.maskfunc, item)

        jones = ma.masked_array(jones_data, mask=jones_mask)
        
        return jones


class _LeakageSlicer(object):
    """Allows sliced view of the leakage solution (see get_leakage)

    2 slices should be supplied for [antenna, beam]

    Returns a tuple containing (d12, d21) where d12 and d21 are numpy masked
    arrays containing the values at the specified slices.
    
    The masks are set as valid if the corresponding IsValid function in the 
    JonesJTerm returns True.
    """
    def __init__(self, func):
        self.func = func

    def __getitem__(self, item):
        d12term = lambda ant, beam: self.func(ant, beam).d12
        d21term = lambda ant, beam: self.func(ant, beam).d21
        d12valid = lambda ant, beam: not self.func(ant, beam).d12IsValid
        d21valid = lambda ant, beam: not self.func(ant, beam).d21IsValid
        
        d12_cube = _func_iterator2(d12term, item)
        d21_cube = _func_iterator2(d21term, item)
        d12valid_cube = _func_iterator2(d12valid, item)
        d21valid_cube = _func_iterator2(d21valid, item)

        d12 = ma.masked_array(d12_cube, mask=d12valid_cube)
        d21 = ma.masked_array(d21_cube, mask=d21valid_cube)
        return d12, d21

class Accessor(object):
    """Accesses calibration solutions

    Solutions can be obtained a single-object at a time using:
    - the get_* functions.
    - sliced views to return masked numpy arrays
    
    3 dimensional views (antenna, beam, channel):
    
    * :func:`bandpass` - returns a tuple containing (g1, g2) as masked arrays

    * :func:`jones` - returns a Nant x Nbeam x Nchan x 2 x 2 masked array containing the full Jones matrix

    2 dimensional views (antenna, beam):

    * :func:`gain` - returns a tuple containing (g1, g2) as masked arrays

    * :func:`leakage` - returns a tuple containing (d12, d21) as masked arrays

    e.g.
    
    >>> ac = Source(...)
    >>> bp_all = ac.bandpass[:, :, :]    # Gets all bandpass data
    >>> g1_ant1_beam2 = ac.gain[1, 2][0] # Gets g1 for ant1, beam2
    >>> g2_ant1_beam2 = ac.gain[0, 3][1] # Gets g2 for ant0, beam3
    """
    
    def __init__(self, accessor):
        self._accessor = accessor
        self._bandpass = _BandpassSlicer(self.get_bandpass)
        self._gain = _GainSlicer(self.get_gain)
        self._leakage = _LeakageSlicer(self.get_leakage)
        self._jones = _JonesSlicer(self.get_jones, self.get_jones_valid)
        
    def get_jones(self, ant, beam, chan):
        """
        Obtain full 2x2 Jones Matrix taking all effects into account
        This method returns resulting 2x2 matrix taking gain, leakage and
        bandpass effects (for a given channel) into account. Invalid gains (and bandpass
        values) are replaced by 1., invalid leakages are replaced by zeros.
        
        The relation between leakage terms and Jones matrices matches 
        the definition of Hamaker, Bregman & Sault. See their equation 
        (14) for details. Our parameters d12 (corresponding to Stokes:XY) and
        d21 (corresponding to Stokes::YX) correspond to d_{Ap} and d_{Aq} from
        Hamaker, Bregman & Sault, respectively. It is assumed that the gain errors
        are applied after leakages (i.e. R=GD).  

        :param ant: Antenna index
        :param beam: Beam index
        :param chan: Channel index
        :return: 2x2 numpy array containing the jones matrix
        """
        jones = np.array(self._accessor.jones(ant, beam, chan), dtype=np.complex)
        jones.shape = (2, 2)
        return jones

    def get_jones_valid(self, ant, beam, chan):
        """Obtain validity flag for full 2x2 Jones Matrix

        :param ant: Antenna index
        :param beam: Beam index
        :param chan: Channel index
        :return: bolean true if valid
        """
        return self._accessor.jonesValid(ant, beam, chan)
        
    def get_gain(self, ant, beam):
        """
        Obtain gains (J-Jones)
        This method retrieves parallel-hand gains for both 
        polarisations (corresponding to XX and YY). If no gains are defined
        for a particular index, gains of 1. with invalid flags set are
        returned.

        :param ant: Antenna index
        :param beam: beam index
        :return: JonesJTerm object with gains and validity flags
        """
        return self._accessor.gain(ant, beam)

    def get_leakage(self, ant, beam):
        """
        Obtain leakage (D-Jones)
        This method retrieves cross-hand elements of the 
        Jones matrix (polarisation leakages). There are two values
        (corresponding to XY and YX) returned (as members of JonesDTerm 
        class). If no leakages are defined for a particular index,
        zero leakages are returned with invalid flags set. 

        :param ant: Antenna index
        :param beam: beam index
        :return: JonesDTerm object with leakages and validity flags
        """
        return self._accessor.leakage(ant, beam)

    def get_bandpass(self, ant, beam, chan):
        """
        Obtain bandpass (frequency dependent J-Jones)
        This method retrieves parallel-hand spectral
        channel-dependent gain (also known as bandpass) for a
        given channel and antenna/beam. The actual implementation
        does not necessarily store these channel-dependent gains
        in an array. It could also implement interpolation or 
        sample a polynomial fit at the given channel (and 
        parameters of the polynomial could be in the database). If
        no bandpass is defined (at all or for this particular channel),
        gains of 1.0 are returned (with invalid flag is set).
        
        :param ant: Antenna index
        :param beam: Beam index
        :param chan: Channel index
        :return: JonesJTerm object with gains and validity flags
        """
        return self._accessor.bandpass(ant, beam, chan)

    @property
    def bandpass(self):
        """Allows sliced view of the bandpass solution (see get_bandpass)
        3 slices should be supplied for [antenna, beam, channel]
        Returns tuple (g1, g2) where g1 and g2 are  3D masked numpy arrays
        [nant, nbeam, nchan], for the g1 and g2 terms.
        The masks are set as valid if the corresponding IsValid function in the 
        JonesDTerm returns True.
        """
        return self._bandpass

    @property
    def leakage(self):
        """Allows sliced view of the leakage solution (see get_leakage)
        2 slices should be supplied for [antenna, beam]
        Returns a tuple containing (d12, d21) where d12 and d21 are numpy masked
        arrays containing the values at the specified slices.
        The masks are set as valid if the corresponding IsValid function in the 
        JonesJTerm returns True.
        """
        return self._leakage

    @property
    def gain(self):
        """Alows sliced view of the gain parameters (see get_gain)
        
        2 Slices should be supplied for [antenna, beam]
        
        Returns a tuple containing (g1, g2) where g1 and g2 are numpy masked
        arrays contaiging the values at the specified slices.
        
        The masks are set as valid if the corresponding IsValid function in the 
        JonesDTerm returns True.
        """

        return self._gain

    @property
    def jones(self):
        """Allows sliced view of the full Jones matrix (see get_jones)
        3 slices should be supplied for [antenna, beam, channel]
        Returns a 5D masked numpy array: [nant, nbeam, nchan, 2, 2] where the final 2
        dimensions are the Jones matrix for the given antenna and beam.
        The mask for all Jones terms are set as valid if jonesValid() returns true
        for the given antenna, beam and channel.
        """
        return self._jones
