/// @file UVChannelDataConverter.h
/// @brief An implementaion of the IDataConverter interface for the uv-channel.
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

#ifndef ASKAP_CP_CHANNELS_UVCHANNEL_DATA_CONVERTER_H
#define ASKAP_CP_CHANNELS_UVCHANNEL_DATA_CONVERTER_H

// ASKAPsoft includes
#include "dataaccess/BasicDataConverter.h"
#include "casa/aipstype.h"
#include "measures/Measures/MEpoch.h"
#include "measures/Measures/MFrequency.h"
#include "measures/Measures/MDirection.h"
#include "measures/Measures/MRadialVelocity.h"

namespace askap {
namespace cp {
namespace channels {

/// @brief An implementation of the IDataConverter interface for visibility
/// streams.
///
/// @details This converter is not fully implemented and only support the
/// bare minimum required by the prototype streaming imager. Currently it
/// works like so:
/// - setEpochFrame() is not supported and throws an exception.
/// - setDirectionFrame() only supports MDirection::Ref of J2000, otherwise
///   it throws an exception.
/// - setFrequencyFrame() only supports MFrequency::Ref of TOPO and units
///   of "Hz" otherwise it throws an exception.
/// - setVelocityFrame() is not supported and throws an exception.
/// - setRestFrequency() is not supported and throws an exception.
///
class UVChannelDataConverter : virtual public askap::synthesis::BasicDataConverter {
    public:
        UVChannelDataConverter();

        virtual void setEpochFrame(const casa::MEpoch &origin = casa::MEpoch(),
                                   const casa::Unit &unit = "s");


        virtual void setDirectionFrame(const casa::MDirection::Ref &ref,
                                       const casa::Unit &unit = "rad");


        virtual void setFrequencyFrame(const casa::MFrequency::Ref &ref,
                                       const casa::Unit &unit = "Hz");


        virtual void setVelocityFrame(const casa::MRadialVelocity::Ref &ref,
                                      const casa::Unit &unit = "km/s");

        virtual void setRestFrequency(const casa::MVFrequency &restFreq);
};

} // end of namespace channels
} // end of namespace cp
} // end of namespace askap

#endif
