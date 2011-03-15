/// @file UVChannelConstDataIterator.h
/// @brief An implementation of the IConstDataIterator interface for the
/// visibility stream.
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

#ifndef ASKAP_CP_CHANNELS_UVCHANNEL_CONST_DATA_ITERATOR_H
#define ASKAP_CP_CHANNELS_UVCHANNEL_CONST_DATA_ITERATOR_H

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "boost/scoped_ptr.hpp"
#include "dataaccess/IConstDataIterator.h"
#include "dataaccess/IDataSelector.h"
#include "dataaccess/IConstDataAccessor.h"

// Local includes
#include "uvchannel/UVChannelConfig.h"
#include "uvchannel/uvdataaccess/UVChannelConstDataSource.h"
#include "uvchannel/uvdataaccess/UVChannelDataSelector.h"
#include "uvchannel/uvdataaccess/UVChannelDataConverter.h"
#include "uvchannel/uvdataaccess/UVChannelConstDataAccessor.h"
#include "uvchannel/uvdataaccess/UVChannelReceiver.h"

namespace askap {
namespace cp {
namespace channels {

/// @brief An implementation of the IConstDataIterator interface for the
/// uv-channel.
class UVChannelConstDataIterator : virtual public askap::accessors::IConstDataIterator {
    public:
        UVChannelConstDataIterator(const UVChannelConfig& channelConfig,
                                   const std::string& channelName,
                                   const boost::shared_ptr<const UVChannelDataSelector>& sel,
                                   const boost::shared_ptr<const UVChannelDataConverter>& conv);

        virtual void init();

        virtual const askap::accessors::IConstDataAccessor& operator*() const;

        virtual casa::Bool hasMore() const throw();

        virtual casa::Bool next();

    protected:
        const UVChannelConfig itsChannelConfig;
        const std::string itsChannelName;
        const boost::shared_ptr<const UVChannelDataSelector> itsSelector;
        const boost::shared_ptr<const UVChannelDataConverter> itsConverter;
        boost::scoped_ptr<UVChannelReceiver> itsReceiver;

        boost::scoped_ptr<UVChannelConstDataAccessor> itsAccessor;
};

} // end of namespace channels
} // end of namespace cp
} // end of namespace askap

#endif
