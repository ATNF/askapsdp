/// @file MonitoringProviderImpl.h
///
/// @copyright (c) 2014 CSIRO
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

#ifndef ASKAP_CP_INGEST_MONITORINGPROVIDERIMPL_H
#define ASKAP_CP_INGEST_MONITORINGPROVIDERIMPL_H

// ASKAPsoft includes
#include "Ice/Ice.h"

// Ice interfaces
#include "MonitoringProvider.h"

// Local package includes
#include "monitoring/DataManager.h"

namespace askap {
namespace cp {
namespace ingest {

/// This class implements the "MonitoringProvider" Ice interface.
/// Remote clients invoke methods on this class to fetch monitoring data.
class MonitoringProviderImpl : public askap::interfaces::monitoring::MonitoringProvider {
    public:
        /// @brief Constructor.
        MonitoringProviderImpl(DataManager& datasource);

        /// @brief Destructor.
        virtual ~MonitoringProviderImpl();

        /// The caller provides zero or more point names and the
        /// return value will contain at most the same number of
        /// monitoring points in the returned sequence.
        ///
        /// Where a point name is not available it will simply not
        /// be included in the result sequence.
        ///
        /// If an empty sequence is passed, the returned sequence will
        /// be empty.
        ///
        /// @param[in] pointnames   a sequence of point names for
        ///                         which data will be fetched
        virtual askap::interfaces::monitoring::MonitorPointSeq get(
                const askap::interfaces::StringSeq& pointnames,
                const Ice::Current&);

    private:
        // Source of monitoring data
        DataManager& itsDataSource;

        // No support for assignment
        MonitoringProviderImpl& operator=(const MonitoringProviderImpl& rhs);

        // No support for copy constructor
        MonitoringProviderImpl(const MonitoringProviderImpl& src);
};

}
}
}

#endif
