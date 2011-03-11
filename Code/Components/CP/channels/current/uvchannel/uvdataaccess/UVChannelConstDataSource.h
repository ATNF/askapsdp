/// @file UVChannelConstDataSource.h
/// @brief An implementation of IConstDataSource for streamed visibility data.
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

#ifndef ASKAP_CP_CHANNELS_UVCHANNEL_CONST_DATA_SOURCE_H
#define ASKAP_CP_CHANNELS_UVCHANNEL_CONST_DATA_SOURCE_H

// System includes
#include <string>

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "dataaccess/IConstDataSource.h"
#include "Common/ParameterSet.h"
#include "uvchannel/UVChannelConfig.h"

namespace askap {
namespace cp {
namespace channels {

/// @brief An implementation of IConstDataSource for streamed visibility data.
class UVChannelConstDataSource : virtual public askap::accessors::IConstDataSource {
    public:
        /// @brief Construct a read-only data source object.
        /// @param[in] the source VisStreamObject.
        /// @param[out] channel name for data channel of interest. This must be
        ///             one of the channel names described in the parset.
        UVChannelConstDataSource(const LOFAR::ParameterSet& parset,
                                 const std::string& channelName);

        /// @brief Create a converter object corresponding to this type
        /// of the DataSource.
        ///
        /// The user can change converting policies (units,
        /// reference frames) by appropriate calls to this converter object
        /// and pass it back to createConstIterator(...). The data returned by
        /// the iterator will automatically be in the requested frame/units
        ///
        /// @return a shared pointer to a new DataConverter object
        ///
        /// The method acts as a factory by creating a new DataConverter.
        /// The lifetime of this converter is the same as the lifetime of the
        /// DataSource object. Therefore, it can be reused multiple times,
        /// if necessary. However, the behavior of iterators created
        /// with a particular DataConverter is undefined, if you change
        /// the DataConverter after the creation of an iterator, unless you
        /// call init() of the iterator (and start a new iteration loop).
        virtual askap::accessors::IDataConverterPtr createConverter() const;

        /// @brief obtain a read-only iterator
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
        /// if necessary. Call init() to rewind the iterator.
        virtual boost::shared_ptr<askap::accessors::IConstDataIterator> createConstIterator(
            const askap::accessors::IDataSelectorConstPtr &sel,
            const askap::accessors::IDataConverterConstPtr &conv) const;

        /// @brief Create a selector object corresponding to this type of the
        /// DataSource.
        ///
        /// @return a shared pointer to the DataSelector corresponding to
        /// this type of DataSource. DataSource acts as a factory and
        /// creates a selector object of the appropriate type
        ///
        /// This method acts as a factory by creating a new DataSelector
        /// appropriate to the given DataSource. The lifetime of the
        /// DataSelector is the same as the lifetime of the DataSource
        /// object. Therefore, it can be reused multiple times,
        /// if necessary. However, the behavior of iterators already obtained
        /// with this DataSelector is undefined, if one changes the selection
        /// unless the init method is called for the iterator (and the new
        /// iteration loop is started).
        virtual askap::accessors::IDataSelectorPtr createSelector() const;

    private:
        const UVChannelConfig itsChannelConfig;
        const std::string itsChannelName;
};

} // namespace channels
} // namespace cp
} // namespace askap

#endif
