/// @file SkyModelServiceClient.h
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

#ifndef ASKAP_CP_SKYMODELSERVICE_SKYMODELSERVICECLIENT_H
#define ASKAP_CP_SKYMODELSERVICE_SKYMODELSERVICECLIENT_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "Ice/Ice.h"
#include "SkyModelService.h" // Ice generated interface
#include "casa/Quanta/Quantum.h"

// Local package includes
#include "skymodelclient/Component.h"
#include "skymodelclient/ComponentResultSet.h"

namespace askap {
namespace cp {
namespace skymodelservice {

    class SkyModelServiceClient {

        public:
            /// Constructor
            /// The three parameters passed allow an instance of the sky model
            /// service to be located in an ICE registry.
            ///
            /// @param[in] locatorHost  host of the ICE locator service.
            /// @param[in] locatorPort  port of the ICE locator service.
            /// @param[in] serviceName  identity of the calibration data service
            ///                         in the ICE registry.
            SkyModelServiceClient(const std::string& locatorHost,
                    const std::string& locatorPort,
                    const std::string& serviceName = "SkyModelService");

            /// Destructor.
            ~SkyModelServiceClient();

            /// Temporary method, to be replaced by a method allowing updating of the GSM
            /// from an updated LSM.
            std::vector<ComponentId> addComponents(const std::vector<Component>& components);

            /// Cone search.
            ///
            /// @param ra   the right ascension of the centre of the
            ///             search area (Unit conformance: decimal degrees).
            /// @param dec  the declination of the centre of the search
            ///              area (Unit conformance: decimal degrees).
            /// @param searchRadius the search radius (Unit conformance:
            ///                      decimal degrees).
            /// @param fluxLimit    low limit on flux on sources returned all
            ///                     returned sources shall have flux >= fluxLimit
            ///                     (Unit conformance: Jy).
            /// @throw  AskapError  in the case one ore more of the Quantities does not
            ///                     conform to the appropriate unit.
            ComponentResultSet coneSearch(const casa::Quantity& ra,
                    const casa::Quantity& dec,
                    const casa::Quantity& searchRadius,
                    const casa::Quantity& fluxLimit);

        private:

            // Ice Communicator
            Ice::CommunicatorPtr itsComm;

            // Proxy object for remote service
            askap::interfaces::skymodelservice::ISkyModelServicePrx itsService;

            // No support for assignment
            SkyModelServiceClient& operator=(const SkyModelServiceClient& rhs);

            // No support for copy constructor
            SkyModelServiceClient(const SkyModelServiceClient& src);
    };

};
};
};

#endif
