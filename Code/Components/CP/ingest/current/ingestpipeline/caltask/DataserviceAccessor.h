/// @file DataserviceAccessor.h
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

#ifndef ASKAP_CP_INGEST_DATASERVICEACCESSOR_H
#define ASKAP_CP_INGEST_DATASERVICEACCESSOR_H

// System includes
#include <string>

// ASKAPsoft includes
#include "casa/aipstype.h"
#include "casa/BasicSL/Complex.h"

// Local package includes
#include "ingestpipeline/caltask/ISolutionAccessor.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief An interface for accessing calibration solutions.
class DataserviceAccessor : public ISolutionAccessor {
    public:
        /// Destructor.
        DataserviceAccessor(const std::string& locatorHost,
                            const std::string& locatorPort,
                            const std::string& serviceName);

        /// Destructor.
        virtual ~DataserviceAccessor();

        /// @see ISolutionAccessor::getGain
        virtual casa::Complex getGain(casa::uInt ant,
                                      casa::uInt beam,
                                      ISolutionAccessor::Pol pol,
                                      casa::Bool& valid) const;

        /// @see ISolutionAccessor::getLeakage
        virtual casa::Complex getLeakage(casa::uInt ant,
                                         casa::uInt beam,
                                         ISolutionAccessor::LeakageTerm term,
                                         casa::Bool& valid) const;

        /// @see ISolutionAccessor::getBandpass
        virtual casa::Complex getBandpass(casa::uInt ant,
                                          casa::uInt beam,
                                          casa::uInt chan,
                                          ISolutionAccessor::Pol pol,
                                          casa::Bool& valid) const;
};

}
}
}

#endif
