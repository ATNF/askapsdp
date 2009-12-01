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
     * This visibilities structure exists to encapsulate the raw binary
     * visibilities payload and allow it to be published via an IceStorm topic.
     * This structure however does contain a timestamp and coarse channel
     * number to provide basic identification of the payload. Such information
     * can be used for routing of the structure.
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
         * Payload - Raw binary payload
         **/
        askap::interfaces::ByteSeq payload;
    };

    /**
     * The interface to  the Visibilities stream
     **/
    interface IVisStream
    {
        void publish(Visibilities vis);
    };

}; // End module cp
}; // End module interfaces
}; // End module askap

#endif
