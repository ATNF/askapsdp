// @file CP.ice
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

#ifndef ASKAP_CP_ICE
#define ASKAP_CP_ICE

#include <CommonTypes.ice>

module askap
{

module interfaces
{

module cp
{
    /////////////////////////////////////////////////////////////////
    // Observing Service
    /////////////////////////////////////////////////////////////////
    
    interface ICPObsService
    {
        ["ami"] void startObs(askap::interfaces::ParameterMap parmap);
        ["ami"] void abortObs();
    };

    /////////////////////////////////////////////////////////////////
    // Visiblitiy Stream
    /////////////////////////////////////////////////////////////////

    /**
     * Visibilities
     *
     * Big Theory Statement
     * --------------------
     * This structure represents a block of visibilities. One instance of this
     * structure contains all the correlations for a single coarse channel (given
     * by coarseChannel) for a single integration (given by timestamp). This
     * structure aims to encapsulate the output from the correlator and
     * aims to be as self-describing as possible, however must work
     * with the limited amount of information that is available on the correlator
     * control computer (CCC).
     *
     * Baseline Ordering - The ordering of the baselines in the array is arbitary,
     * and as such the antenna1 and antenna2 elements are needed to map a 
     * correlation back to its antenna pair.
     *
     * Polarization Ordering - The polarization products are guaranteed to be in
     * order I, Q, U, V
     *
     * Channel Ordering - The fine channels within this structure are guaranteed
     * to be in sequence in ascending order.
     *
     * Beam Ordering - The ordering of the beams in the array is arbitary,
     * and as such the beam1 and beam2 elements are needed to map a 
     * correlation back to its beam pair. Note, for ASKAP it is expected
     * that beam1 == beam2 will always be true for a given correlation.
     *
     **/ 
    struct Visibilities {
        /**
         * Timestamp - Binary Atomic Time (BAT). The number of microseconds
         * since Modified Julian Day (MJD) = 0
         **/
        long timestamp;

        /**
         * Coarse Channel - Which coarse channel this block of data relates to.
         **/
        int coarseChannel;

        /**
         * Number of baselines in the visArray.
         **/
        int nBaselines;

        /**
         * Number of polarisation products in the visArray.
         **/
        int nPols;

        /**
         * Number of channels in the visArray.
         **/
        int nChannels;

        /**
         * Number of beams in the visArray.
         **/
        int nBeams;

        /**
         * A sequence of length nBaselines, indicating the ID of the FIRST
         * antenna of the pair;
         **/
        askap::interfaces::IntSeq antenna1;

        /**
         * A sequence of length nBaselines, indicating the ID of the SECOND
         * antenna of the pair;
         **/
        askap::interfaces::IntSeq antenna2;

        /**
         * A sequence of length nBeams, indicating the ID of the FIRST
         * beam of the pair;
         **/
        askap::interfaces::IntSeq beam1;

        /**
         * A sequence of length nBeams, indicating the ID of the SECOND
         * beam of the pair;
         **/
        askap::interfaces::IntSeq beam2;

        /**
         *  Data - Single dimensional array/sequence of length:
         *      nBaselines * nPols * nChannels * nBeams
         *
         * The following is an example of how one would index into this array.
         * All the parameters and the return from this function is zero based.
         * <pre>
         * int index(int baseline, int pol, int chan, int beam) {
         *      return baseline + ((nBaselines) * pol)
         *              + ((nBaselines * nPols) * chan)
         *              + ((nBaselines * nPols * nChannels) * beam);
         * }
         *
         * </pre>
         *
         * Or in terms of a multi-dimensional C array stored in linear memory
         * (row-major order):
         * <pre>
         * FloatComplex array[nBeams][nChannels][nPols][nBaselines]; 
         * </pre>
         *
         **/
        askap::interfaces::FloatComplexSeq visArray;
    };

    /**
     * The interface to implement when receiving VisCubes
     **/
    interface IVisStream
    {
        void publish(Visibilities vis);
    };

}; // End module cp
}; // End module interfaces
}; // End module askap

#endif
