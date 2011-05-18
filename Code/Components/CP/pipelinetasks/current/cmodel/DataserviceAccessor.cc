/// @file DataserviceAccessor.cc
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
#include "cmodel/DataserviceAccessor.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <string>
#include <vector>

// ASKAPsoft include
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "skymodelclient/ComponentResultSet.h"
#include "skymodelclient/Component.h"

// Casacore includes
#include "casa/aipstype.h"
#include "casa/Quanta/Quantum.h"

// Using
using namespace casa;
using namespace askap;
using namespace askap::cp::pipelinetasks;
using namespace askap::cp::skymodelservice;

ASKAP_LOGGER(logger, ".DataserviceAccessor");

DataserviceAccessor::DataserviceAccessor(const std::string& locatorHost,
        const std::string& locatorPort,
        const std::string& serviceName = "SkyModelService")
    : itsService(locatorHost, locatorPort, serviceName)
{
}

DataserviceAccessor::~DataserviceAccessor()
{
}

std::vector<askap::cp::skymodelservice::Component> DataserviceAccessor::coneSearch(const casa::Quantity& ra,
        const casa::Quantity& dec,
        const casa::Quantity& searchRadius,
        const casa::Quantity& fluxLimit)
{
    // Pre-conditions
    ASKAPCHECK(ra.isConform("deg"), "ra must conform to degrees");
    ASKAPCHECK(dec.isConform("deg"), "dec must conform to degrees");
    ASKAPCHECK(searchRadius.isConform("deg"), "searchRadius must conform to degrees");
    ASKAPCHECK(fluxLimit.isConform("Jy"), "fluxLimit must conform to Jy");

    ASKAPLOG_DEBUG_STR(logger, "Cone search - ra: " << ra.getValue("deg")
                           << " deg, dec: " << dec.getValue("deg")
                           << " deg, radius: " << searchRadius.getValue("deg")
                           << " deg, Fluxlimit: " << fluxLimit.getValue("Jy") << " Jy");


    ComponentResultSet rs = itsService.coneSearch(ra, dec, searchRadius, fluxLimit);
    ComponentResultSet::Iterator it = rs.createIterator();
    std::vector<askap::cp::skymodelservice::Component> list;
    while (it.hasNext()) {
        it.next();
        list.push_back(*it);
    }

    // Post-conditions
    ASKAPCHECK(list.size() == rs.size(), "Component list size mismatch");

    return list;
}

