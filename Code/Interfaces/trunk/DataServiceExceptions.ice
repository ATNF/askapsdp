// @file DataServiceExceptions.ice
//
// @copyright (c) 2010 CSIRO
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

#ifndef ASKAP_DATASERVICEEXCEPTIONS_ICE
#define ASKAP_DATASERVICEEXCEPTIONS_ICE

#include <CommonTypes.ice>

module askap
{

module interfaces
{

module schedblock
{

    /**
     * This exception is thrown when a state transition fails. The base
     * class includes a "reason" data member (of type string) which shall
     * be used to indicate why the state transition failed.
     **/
    exception TransitionException extends askap::interfaces::AskapIceException
    {
    };

    /**
     * This exceptions is thrown when a ParameterMap with illegal values is
     * passed into the method.
     **/
    exception ParameterException extends askap::interfaces::AskapIceException
    {
    };

    /** This exceptions is thrown when the given Scheduling Block id doesn't
     * exists.
     **/
    exception NoSuchSchedulingBlockException
        extends askap::interfaces::AskapIceException
    {
    };

    /** This exceptions is thrown when the given Observation Program id doesn't
     * exists.
     **/
    exception NoSuchProgramException
        extends askap::interfaces::AskapIceException
    {
    };

    /** This exceptions is thrown when the given Software Instrument id doesn't
     * exists.
     **/
    exception NoSuchSoftwareInstrumentException
        extends askap::interfaces::AskapIceException
    {
    };

};
};
};

#endif
