// @file CommonTypes.ice
//
// @copyright (c) 2009 CSIRO
// Australia Telescope National Facility (ATNF)
// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
// PO Box 76, Epping NSW 1710, Australia
// atnf-enquiries@csiro.au
//
// This file is part of the ASKAP software distribution.
//
// The ASKAP software distribution is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

#ifndef ASKAP_COMMONTYPES_ICE
#define ASKAP_COMMONTYPES_ICE

module askap
{

module interfaces
{
    /**
     * Base exception from which all ICE exceptions thrown by
     * ASKAPsoft code should derive from. The reason string shall be
     * used to indicate why the exception was thrown
     **/
    exception AskapIceException
    {
        string reason;
    };

    /**
     * This is a common type which can be used as the Ice equivalent of the
     * LOFAR ParameterSet. This contains a map of variable names and maps
     * them to their value.
     **/
    dictionary<string, string> ParameterMap;

    /** A sequence of bools. **/
    sequence<bool> BoolSeq;

    /** A sequence of bytes. **/
    sequence<byte> ByteSeq;

    /** A sequence of shorts. **/
    sequence<short> ShortSeq;

    /** A sequence of ints. **/
    sequence<int> IntSeq;

    /** A sequence of longs. **/
    sequence<long> LongSeq;

    /** A sequence of floats. **/
    sequence<float> FloatSeq;

    /** A sequence of doubles. **/
    sequence<double> DoubleSeq;

    /** A sequence of strings. **/
    sequence<string> StringSeq;

    /**
     * A single-precision complex number.
     **/
    struct FloatComplex{
        float real;
        float imag;
    };

    /**
     * A sequence of single-precision complex numbers.
     **/
    sequence<FloatComplex> FloatComplexSeq;

    /**
     * A double-precision complex number.
     **/
    struct DoubleComplex {
        double real;
        double imag;
    };

    /**
     * A sequence of double-precision complex numbers.
     **/
    sequence<DoubleComplex> DoubleComplexSeq;

    /**
     * Astronomical direction (a measure)
     */
    enum CoordSys { J2000, AZEL };
    struct Direction {
        double coord1;
        double coord2;
        CoordSys sys;
    };

    /**
     * A sequence of astronomical directions
     */
    sequence<Direction> DirectionSeq;

}; // End module interfaces
}; // End module askap

#endif
