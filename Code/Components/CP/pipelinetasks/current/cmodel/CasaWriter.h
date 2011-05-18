/// @file CasaWriter.h
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

#ifndef ASKAP_CP_PIPELINETASKS_ICASAWRITER_H
#define ASKAP_CP_PIPELINETASKS_ICASAWRITER_H

// System includes
#include <vector>

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "skymodelclient/Component.h"

// Casacore includes
#include "casa/aipstype.h"
#include "components/ComponentModels/ComponentList.h"
#include "coordinates/Coordinates/CoordinateSystem.h"

namespace askap {
namespace cp {
namespace pipelinetasks {

/// @brief An instance of ILocalSkyModelWriter supporting writing the LSM
/// to a CASA image.
class CasaWriter {
    public:
        /// Constructor
        CasaWriter(const LOFAR::ParameterSet& parset);

        /// Creates and writes out an image generated from the component list.
        /// @note The image parameters are read from the itsParset
        /// member.
        ///
        /// @param[in] component    the component list from which the image is
        ///                         generated.
        void write(const std::vector<askap::cp::skymodelservice::Component> components);

    private:

        casa::ComponentList translateComponentList(const std::vector<askap::cp::skymodelservice::Component> components);

        // Create a coordinate system
        // @note The image parameters are read from the itsParset
        casa::CoordinateSystem createCoordinateSystem(casa::uInt nx, casa::uInt ny);

        // Parameter set
        const LOFAR::ParameterSet itsParset;
};

}
}
}

#endif
