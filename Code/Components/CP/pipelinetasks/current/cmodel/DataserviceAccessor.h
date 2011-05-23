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

#ifndef ASKAP_CP_PIPELINETASKS_DATASERVICEACCESSOR_H
#define ASKAP_CP_PIPELINETASKS_DATASERVICEACCESSOR_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "casa/Quanta/Quantum.h"
#include "skymodelclient/Component.h"
#include "skymodelclient/SkyModelServiceClient.h"

// Local package includes
#include "cmodel/IGlobalSkyModel.h"

namespace askap {
namespace cp {
namespace pipelinetasks {

class DataserviceAccessor : public IGlobalSkyModel {
    public:
        /// Constructor
        DataserviceAccessor(const std::string& locatorHost,
                            const std::string& locatorPort,
                            const std::string& serviceName);

        // Destructor
        ~DataserviceAccessor();

        // Conesearch (or filter)
        virtual std::vector<askap::cp::skymodelservice::Component> coneSearch(const casa::Quantity& ra,
                const casa::Quantity& dec,
                const casa::Quantity& searchRadius,
                const casa::Quantity& fluxLimit);

    private:
        askap::cp::skymodelservice::SkyModelServiceClient itsService;
};

}
}
}

#endif
