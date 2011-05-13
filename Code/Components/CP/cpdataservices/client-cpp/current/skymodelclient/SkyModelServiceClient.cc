/// @file SkyModelServiceClient.cc
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

// Include own header file first
#include "SkyModelServiceClient.h"

// Include package level header file
#include "askap_cpdataservices.h"

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "Ice/Ice.h"
#include "iceutils/CommunicatorConfig.h"
#include "iceutils/CommunicatorFactory.h"
#include "SkyModelService.h" // Ice generated interface
#include "casa/Quanta/Quantum.h"

// Local package includes
#include "skymodelclient/ComponentResultSet.h"

// Using
using namespace std;
using namespace askap;
using namespace askap::cp::skymodelservice;

SkyModelServiceClient::SkyModelServiceClient(const std::string& locatorHost,
                            const std::string& locatorPort,
                            const std::string& serviceName)
{
    askap::cp::icewrapper::CommunicatorConfig config(locatorHost, locatorPort);
    config.setProperty("Ice.MessageSizeMax", "131072");
    askap::cp::icewrapper::CommunicatorFactory commFactory;
    itsComm = commFactory.createCommunicator(config);

    ASKAPDEBUGASSERT(itsComm);

    Ice::ObjectPrx base = itsComm->stringToProxy(serviceName);
    itsService = askap::interfaces::skymodelservice::ISkyModelServicePrx::checkedCast(base);

    if (!itsService) {
        ASKAPTHROW(AskapError, "SkyModelService proxy is invalid");
    }
}

SkyModelServiceClient::~SkyModelServiceClient()
{
}

std::vector<ComponentId> SkyModelServiceClient::addComponents(const std::vector<Component>& components)
{
    askap::interfaces::skymodelservice::ComponentSeq ice_components;
    for (size_t i = 0; i < components.size(); ++i) {
        askap::interfaces::skymodelservice::Component ice_component;

        ice_component.id = components[i].id();
        ice_component.rightAscension = components[i].rightAscension().getValue("deg");
        ice_component.declination = components[i].declination().getValue("deg");
        ice_component.positionAngle = components[i].positionAngle().getValue("rad");
        ice_component.majorAxis = components[i].majorAxis().getValue("arcsec");
        ice_component.minorAxis = components[i].minorAxis().getValue("arcsec");
        ice_component.i1400 = components[i].i1400().getValue("Jy");

        ice_components.push_back(ice_component);
    }

    askap::interfaces::skymodelservice::ComponentIdSeq ice_ids = itsService->addComponents(ice_components);

    std::vector<ComponentId> ids(ice_ids.size());
    for (size_t i = 0; i < ids.size(); ++i) {
        ids[i] = ice_ids[i];
    }

    return ids;
}

ComponentResultSet SkyModelServiceClient::coneSearch(const casa::Quantity& ra, 
        const casa::Quantity& dec, const casa::Quantity& searchRadius,
        const casa::Quantity& fluxLimit)
{
    askap::interfaces::skymodelservice::ComponentIdSeq ice_resultset =
        itsService->coneSearch(ra.getValue("deg"), dec.getValue("deg"),
                searchRadius.getValue("deg"), fluxLimit.getValue("Jy"));

    return ComponentResultSet(ice_resultset, itsService);
}
