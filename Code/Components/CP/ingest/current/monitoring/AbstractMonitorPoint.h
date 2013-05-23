/// @file AbstractMonitorPoint.h
///
/// @copyright (c) 2013 CSIRO
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

#ifndef ASKAP_CP_INGEST_ABSTRACTMONITORPOINT_H
#define ASKAP_CP_INGEST_ABSTRACTMONITORPOINT_H

// System includes
#include <string>

// ASKAPsoft includes

// Local package includes
#include "monitoring/MonitoringSingleton.h"

namespace askap {
namespace cp {
namespace ingest {

template <typename T>
class AbstractMonitorPoint {
    public:
        AbstractMonitorPoint(const std::string& name)
                : itsName(name), itsDestination(MonitoringSingleton::instance()) {
        }

        virtual ~AbstractMonitorPoint() {
        }

        virtual void update(const T& value) {
            if (itsDestination) send(itsName, value);
        }

    protected:
        virtual void send(const std::string& name, const T& value) = 0;

        const std::string itsName;
        MonitoringSingleton* itsDestination;
};

} // End namespace ingest
} // End namespace cp
} // End namespace askap

#endif
