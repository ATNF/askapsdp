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

    ASKAPLOG_DEBUG_STR(logger, "Reading VOTable (this may take some time)");
    const VOTable vot = VOTable::fromXML(itsFilename);
    ASKAPCHECK(vot.getResource().size() == 1, "Only a single RESOURCE element is supported");
    ASKAPCHECK(vot.getResource()[0].getTables().size() == 1, "Only a single TABLE element is supported");

    // Initialise the field descriptions
    const std::vector<VOTableField> fields = vot.getResource()[0].getTables()[0].getFields();
    std::map<VOTableAccessor::FieldEnum, size_t> posMap;
    std::map<VOTableAccessor::FieldEnum, casa::Unit> unitMap;
    initFieldInfo(fields, posMap, unitMap);

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

bool VOTableAccessor::hasUCD(const askap::accessors::VOTableField& field, const std::string& ucd)
{
    // Replace ';' with space to make tokenising easy
    string ucdlist(field.getUCD());
    replace(ucdlist.begin(), ucdlist.end(), ';', ' ');

    // Tokenize the line
    stringstream iss(ucdlist);
    vector<string> tokens;
    copy(istream_iterator<string>(iss),
         istream_iterator<string>(),
         back_inserter<vector<string> >(tokens));

    return find(tokens.begin(), tokens.end(), ucd) != tokens.end();
}

void VOTableAccessor::initFieldInfo(const std::vector<askap::accessors::VOTableField>& fields,
        std::map<VOTableAccessor::FieldEnum, size_t>& posMap,
        std::map<VOTableAccessor::FieldEnum, casa::Unit>& unitMap)
{
    // Preconditions
    ASKAPCHECK(fields.size() != 0, "No field descriptions present");

    // Create a mapping between UCD and the FieldEnum
    std::map<std::string, VOTableAccessor::FieldEnum> fmap;
    fmap["pos.eq.ra"] = RA;
    fmap["pos.eq.dec"] = DEC;
    fmap["phot.flux.density"] = FLUX;
    fmap["phys.angSize.smajAxis"] = MAJOR_AXIS;
    fmap["phys.angSize.sminAxis"] = MINOR_AXIS;
    fmap["pos.posAng"] = POSITION_ANGLE;
    fmap["spectral.index"] = SPECTRAL_INDEX;
    fmap["spectral.curvature"] = SPECTRAL_CURVATURE;

    for (size_t i = 0; i < fields.size(); ++i) {
        const VOTableField field = fields[i];

        // Check the field map to see if we care about this field
        std::map<std::string, VOTableAccessor::FieldEnum>::const_iterator fiter;
        for (fiter = fmap.begin(); fiter != fmap.end(); ++fiter) {
            if (hasUCD(field, fiter->first)) {
                if (posMap.find(fiter->second) != posMap.end()) {
                    ASKAPTHROW(AskapError, "The UCD " << fiter->first
                            << " appears in the field list multiple times");
                }
                posMap[fiter->second] = i;
                if (field.getUnit().size() > 0) {
                    unitMap[fiter->second] = Unit(field.getUnit());
                }
            }
        }
    }

    // Post-conditions:
    // Ensure the required fields are present
    ASKAPCHECK(posMap.find(RA) != posMap.end(), "RA field not found");
    ASKAPCHECK(posMap.find(DEC) != posMap.end(), "Dec field not found");
    ASKAPCHECK(posMap.find(FLUX) != posMap.end(), "Flux field not found");
    ASKAPCHECK(posMap.find(MAJOR_AXIS) != posMap.end(), "Major axis field not found");
    ASKAPCHECK(posMap.find(MINOR_AXIS) != posMap.end(), "Minor axis field not found");
    ASKAPCHECK(posMap.find(POSITION_ANGLE) != posMap.end(), "Position angle field not found");
    ASKAPCHECK(posMap.find(SPECTRAL_INDEX) != posMap.end(), "Spectral index field not found");
    ASKAPCHECK(posMap.find(SPECTRAL_CURVATURE) != posMap.end(), "Spectral curvature field not found");
}

void VOTableAccessor::processRow(const std::vector<std::string>& cells,
        const casa::Quantity& searchRA,
        const casa::Quantity& searchDec,
        const casa::Quantity& searchRadius,
        const casa::Quantity& fluxLimit,
        std::map<VOTableAccessor::FieldEnum, size_t>& posMap,
        std::map<VOTableAccessor::FieldEnum, casa::Unit>& unitMap,
        std::list<askap::cp::skymodelservice::Component>& list)
{
    // Create these once to avoid the performance impact of creating them over and over.
    static casa::Unit deg("deg");
    static casa::Unit rad("rad");
    static casa::Unit arcsec("arcsec");
    static casa::Unit Jy("Jy");

    const casa::Quantity ra(boost::lexical_cast<casa::Double>(cells[posMap[RA]]),
            unitMap[RA]);

    const casa::Quantity dec(boost::lexical_cast<casa::Double>(cells[posMap[DEC]]),
            unitMap[DEC]);

    // Discard if outside cone
    const MVDirection searchRefDir(searchRA, searchDec);
    const MVDirection componentDir(ra, dec);
    const Quantity separation = searchRefDir.separation(componentDir, deg);
    if (separation.getValue(deg) > searchRadius.getValue(deg)) {
        itsOutsideSearchCone++;
        return;
    }

    const casa::Quantity flux(boost::lexical_cast<casa::Double>(cells[posMap[FLUX]]),
            unitMap[FLUX]);

    // Discard if below flux limit
    if (flux.getValue(Jy) < fluxLimit.getValue(Jy)) {
        itsBelowFluxLimit++;
        return;
    }

    casa::Quantity majorAxis(boost::lexical_cast<casa::Double>(cells[posMap[MAJOR_AXIS]]),
            unitMap[MAJOR_AXIS]);

    casa::Quantity minorAxis(boost::lexical_cast<casa::Double>(cells[posMap[MINOR_AXIS]]),
            unitMap[MINOR_AXIS]);

    casa::Quantity positionAngle(boost::lexical_cast<casa::Double>(cells[posMap[POSITION_ANGLE]]),
            unitMap[POSITION_ANGLE]);

    double spectralIndex = boost::lexical_cast<casa::Double>(cells[posMap[SPECTRAL_INDEX]]);
    double spectralCurvature = boost::lexical_cast<casa::Double>(cells[posMap[SPECTRAL_CURVATURE]]);

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
