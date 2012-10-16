/// @file VOTableAccessor.cc
///
/// @copyright (c) 2012 CSIRO
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
#include "cmodel/VOTableAccessor.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <vector>
#include <list>

// ASKAPsoft include
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "skymodelclient/Component.h"
#include "votable/VOTable.h"
#include "boost/lexical_cast.hpp"

// Casacore includes
#include "casa/aipstype.h"
#include "casa/Quanta/Quantum.h"
#include "casa/Quanta/MVDirection.h"

// Using
using namespace casa;
using namespace std;
using namespace askap;
using namespace askap::accessors;
using namespace askap::cp::pipelinetasks;

ASKAP_LOGGER(logger, ".VOTableAccessor");

VOTableAccessor::VOTableAccessor(const std::string& filename)
        : itsFilename(filename)
{
}

VOTableAccessor::VOTableAccessor(const std::stringstream& sstream)
{
}

VOTableAccessor::~VOTableAccessor()
{
}

std::vector<askap::cp::skymodelservice::Component> VOTableAccessor::coneSearch(const casa::Quantity& ra,
        const casa::Quantity& dec,
        const casa::Quantity& searchRadius,
        const casa::Quantity& fluxLimit)
{
    ASKAPLOG_INFO_STR(logger, "Cone search - ra: " << ra.getValue("deg")
            << " deg, dec: " << dec.getValue("deg")
            << " deg, radius: " << searchRadius.getValue("deg")
            << " deg, Fluxlimit: " << fluxLimit.getValue("Jy") << " Jy");

    itsBelowFluxLimit = 0;
    itsOutsideSearchCone = 0;

    ASKAPLOG_DEBUG_STR(logger, "Reading VOTable...");
    const VOTable vot = VOTable::fromXML(itsFilename);
    ASKAPCHECK(vot.getResource().size() == 1, "Only a single RESOURCE element is supported");
    ASKAPCHECK(vot.getResource()[0].getTables().size() == 1, "Only a single TABLE element is supported");

    // Initialise the field descriptions
    const std::vector<VOTableField> fields = vot.getResource()[0].getTables()[0].getFields();
    std::map<std::string, size_t> posMap;
    std::map<std::string, casa::Unit> unitMap;

    for (size_t i = 0; i < fields.size(); ++i) {
        posMap[fields[i].getName()] = i;
        unitMap[fields[i].getName()] = Unit(fields[i].getUnit());
    }

    // Initially built as a list to allow efficient growth
    std::list<askap::cp::skymodelservice::Component> list;

    // Process each table row
    unsigned long total = 0;
    const std::vector<VOTableRow> rows = vot.getResource()[0].getTables()[0].getRows();
    for (std::vector<VOTableRow>::const_iterator it = rows.begin(); it != rows.end(); ++it) {
        const std::vector<std::string> cells = it->getCells();
        processRow(cells, ra, dec, searchRadius, fluxLimit, posMap, unitMap, list);

        total++;
        if (total % 100000 == 0) {
            ASKAPLOG_DEBUG_STR(logger, "Processed " << total << " of " << rows.size() << " component entries ");
        }

    }

    ASKAPLOG_INFO_STR(logger, "Sources discarded due to flux threshold: " << itsBelowFluxLimit);
    ASKAPLOG_INFO_STR(logger, "Sourced discarded due to being outside the search cone: " << itsOutsideSearchCone);

    // Returned as a vector to minimise memory usage
    return std::vector<askap::cp::skymodelservice::Component>(list.begin(), list.end());
}

void VOTableAccessor::processRow(const std::vector<std::string>& cells,
        const casa::Quantity& searchRA,
        const casa::Quantity& searchDec,
        const casa::Quantity& searchRadius,
        const casa::Quantity& fluxLimit,
        std::map<std::string, size_t>& posMap,
        std::map<std::string, casa::Unit>& unitMap,
        std::list<askap::cp::skymodelservice::Component>& list)
{
    // Create these once to avoid the performance impact of creating them over and over.
    static casa::Unit deg("deg");
    static casa::Unit rad("rad");
    static casa::Unit arcsec("arcsec");
    static casa::Unit Jy("Jy");

    const casa::Quantity ra(boost::lexical_cast<casa::Double>(cells[posMap["RA"]]),
            unitMap["RA"]);

    const casa::Quantity dec(boost::lexical_cast<casa::Double>(cells[posMap["Dec"]]),
            unitMap["Dec"]);

    // Discard if outside cone
    const MVDirection searchRefDir(searchRA, searchDec);
    const MVDirection componentDir(ra, dec);
    const Quantity separation = searchRefDir.separation(componentDir, deg);
    if (separation.getValue(deg) > searchRadius.getValue(deg)) {
        itsOutsideSearchCone++;
        return;
    }

    const casa::Quantity flux(boost::lexical_cast<casa::Double>(cells[posMap["Flux"]]),
            unitMap["Flux"]);

    // Discard if below flux limit
    if (flux.getValue(Jy) < fluxLimit.getValue(Jy)) {
        itsBelowFluxLimit++;
        return;
    }

    casa::Quantity majorAxis(boost::lexical_cast<casa::Double>(cells[posMap["Major axis"]]),
            unitMap["Major axis"]);

    casa::Quantity minorAxis(boost::lexical_cast<casa::Double>(cells[posMap["Minor axis"]]),
            unitMap["Minor axis"]);

    casa::Quantity positionAngle(boost::lexical_cast<casa::Double>(cells[posMap["Position angle"]]),
            unitMap["Position angle"]);

    double spectralIndex = boost::lexical_cast<casa::Double>(cells[posMap["Spectral index"]]);
    double spectralCurvature = boost::lexical_cast<casa::Double>(cells[posMap["Spectral curvature"]]);

    // Ensure major axis is larger than minor axis
    if (majorAxis.getValue() < minorAxis.getValue()) {
        casa::Quantity tmp = minorAxis;
        minorAxis = majorAxis;
        majorAxis = tmp;
    }

    // Ensure if major axis is non-zero, so is the minor axis
    if (majorAxis.getValue() > 0.0 && minorAxis.getValue() == 0.0) {
        minorAxis = casa::Quantity(1.0e-15, arcsec);
    }

    // Build the Component object and add to the list.
    // NOTE: The Component ID has no meaning for this accessor
    askap::cp::skymodelservice::Component c(-1, ra, dec, positionAngle,
            majorAxis, minorAxis, flux, spectralIndex, spectralCurvature);
    list.push_back(c);
}
