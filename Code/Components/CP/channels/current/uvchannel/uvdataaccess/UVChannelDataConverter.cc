/// @file UVChannelDataConverter.cc
/// @brief
///
/// This class is a wrapper around the BasicDataConverter and overrides
/// the methods which are either not implemented at all or only partially
/// implemented. Where a feature is not implemented, an exception is thrown.
///
/// @copyright (c) 2011 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Ben Humphreys <ben.humphreys@csiro.au>

// Include own header file first
#include "UVChannelDataConverter.h"

// ASKAPsoft includes
#include "dataaccess/BasicDataConverter.h"
#include "dataaccess/DataAccessError.h"

// Using
using namespace askap;
using namespace askap::synthesis;
using namespace askap::cp::channels;

UVChannelDataConverter::UVChannelDataConverter()
{
}

void UVChannelDataConverter::setEpochFrame(const casa::MEpoch &origin,
        const casa::Unit &unit)
{
    throw DataAccessLogicError("setEpochFrame() not yet implemented");
}

void UVChannelDataConverter::setDirectionFrame(const casa::MDirection::Ref &ref,
        const casa::Unit &unit)
{
    if (ref.getType() != casa::MDirection::J2000 ||
            unit != "rad") {
        throw DataAccessLogicError("setDirectionFrame() not fully implemented");
    }

    // Call parent
    BasicDataConverter::setDirectionFrame(ref, unit);
}


void UVChannelDataConverter::setFrequencyFrame(const casa::MFrequency::Ref &ref,
        const casa::Unit &unit)
{
    if (ref.getType() != casa::MFrequency::TOPO ||
            unit != "Hz") {
        throw DataAccessLogicError("setFrequencyFrame() not fully implemented");
    }

    // Call parent
    BasicDataConverter::setFrequencyFrame(ref, unit);
}


void UVChannelDataConverter::setVelocityFrame(const casa::MRadialVelocity::Ref &ref,
        const casa::Unit &unit)
{
    throw DataAccessLogicError("setVelocityFrame() not yet implemented");
}

void UVChannelDataConverter::setRestFrequency(const casa::MVFrequency &restFreq)
{
    throw DataAccessLogicError("setRestFrequency() not yet implemented");
}
