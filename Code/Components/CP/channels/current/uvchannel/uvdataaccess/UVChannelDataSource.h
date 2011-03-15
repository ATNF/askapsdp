/// @file UVChannelDataSource.h
/// @brief An implementation of IDataSource for streamed visibility data.
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
///

#ifndef ASKAP_CP_CHANNELS_UVCHANNEL_DATA_SOURCE_H
#define ASKAP_CP_CHANNELS_UVCHANNEL_DATA_SOURCE_H

// System includes
#include <string>

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "dataaccess/IDataSource.h"
#include "dataaccess/IDataIterator.h"
#include "Common/ParameterSet.h"

// Local package includes
#include "uvchannel/UVChannelConfig.h"
#include "uvchannel/uvdataaccess/UVChannelConstDataSource.h"

namespace askap {
namespace cp {
namespace channels {

/// @brief An implementation of IDataSource for streamed visibility data.
class UVChannelDataSource : public UVChannelConstDataSource, virtual public askap::accessors::IDataSource {
    public:
        /// @brief Construct a data source object.
        /// @param[in] parset the parset which describes the channel configuration.
        /// @param[in] channel name for data channel of interest. This must be
        ///             one of the channel names described in the parset.
        UVChannelDataSource(const LOFAR::ParameterSet& parset,
                            const std::string& channelName);

        /// @brief obtain a iterator
        /// @details
        /// get a read/write iterator over a selected part of the dataset
        /// represented by this DataSource object with an explicitly
        /// specified conversion policy.
        ///
        /// @param[in] sel a shared pointer to the selector object defining
        ///            which subset of the data is used
        /// @param[in] conv a shared pointer to the converter object defining
        ///            reference frames and units to be used
        /// @return a shared pointer to DataIterator object
        /// @note
        /// The method acts as a factory by creating a new DataIterator.
        /// The lifetime of this iterator is the same as the lifetime of
        /// the DataSource object. Therefore, it can be reused multiple times,
        /// if necessary.
        virtual boost::shared_ptr<askap::accessors::IDataIterator> createIterator(
            const askap::accessors::IDataSelectorPtr &sel,
            const askap::accessors::IDataConverterPtr &conv) const;
};

} // namespace channels
} // namespace cp
} // namespace askap

#endif
