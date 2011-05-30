/// @file DuchampAccessor.cc
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
#include "cmodel/DuchampAccessor.h"

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

// ASKAPsoft include
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "boost/lexical_cast.hpp"
#include "skymodelclient/Component.h"

// Casacore includes
#include "casa/aipstype.h"
#include "casa/Quanta/Quantum.h"
#include "casa/Quanta/MVDirection.h"

// Using
using namespace casa;
using namespace std;
using namespace askap::cp::pipelinetasks;

ASKAP_LOGGER(logger, ".DuchampAccessor");

DuchampAccessor::DuchampAccessor(const std::string& filename)
        : itsFile(new std::ifstream(filename.c_str()))
{
}

DuchampAccessor::DuchampAccessor(const std::stringstream& sstream)
        : itsFile(new std::stringstream)
{
    *(dynamic_cast<std::stringstream*>(itsFile.get())) << sstream.str();
}

DuchampAccessor::~DuchampAccessor()
{
}

std::vector<askap::cp::skymodelservice::Component> DuchampAccessor::coneSearch(const casa::Quantity& ra,
        const casa::Quantity& dec,
        const casa::Quantity& searchRadius,
        const casa::Quantity& fluxLimit)
{
    ASKAPLOG_INFO_STR(logger, "Cone search - ra: " << ra.getValue("deg")
                           << " deg, dec: " << dec.getValue("deg")
                           << " deg, radius: " << searchRadius.getValue("deg")
                           << " deg, Fluxlimit: " << fluxLimit.getValue("Jy") << " Jy");

    // Seek back to the beginning of the buffer before reading line by line
    itsFile->seekg(0, std::ios::beg);
    std::string line;
    itsBelowFluxLimit = 0;
    itsOutsideSearchCone = 0;
    casa::uLong total = 0;
    std::vector<askap::cp::skymodelservice::Component> list;

    while (getline(*itsFile, line)) {
        if (line.find_first_of("#") == std::string::npos) {
            processLine(line, ra, dec, searchRadius, fluxLimit, list);
            total++;

            if (total % 100000 == 0) {
                ASKAPLOG_DEBUG_STR(logger, "Read " << total << " component entries");
            }
        }
    }

    ASKAPLOG_INFO_STR(logger, "Sources discarded due to flux threshold: " << itsBelowFluxLimit);
    ASKAPLOG_INFO_STR(logger, "Sourced discarded due to being outside the search cone: " << itsOutsideSearchCone);
    return list;
}

void DuchampAccessor::processLine(const std::string& line,
                                  const casa::Quantity& searchRA,
                                  const casa::Quantity& searchDec,
                                  const casa::Quantity& searchRadius,
                                  const casa::Quantity& fluxLimit,
                                  std::vector<askap::cp::skymodelservice::Component>& list)
{
    // Create these once to avoid the performance impact of creating them over and over.
    static casa::Unit deg("deg");
    static casa::Unit rad("rad");
    static casa::Unit arcsec("arcsec");
    static casa::Unit Jy("Jy");

    // Tokenize the line
    stringstream iss(line);
    vector<string> tokens;
    copy(istream_iterator<string>(iss),
         istream_iterator<string>(),
         back_inserter<vector<string> >(tokens));

    // Positions of the tokens of interest
    const std::vector<casa::uShort> positions = getPositions(tokens.size());
    ASKAPDEBUGASSERT(positions.size() == 6);
    const casa::uShort raPos = positions[0];
    const casa::uShort decPos = positions[1];
    const casa::uShort fluxPos = positions[2];
    const casa::uShort majorAxisPos = positions[3];
    const casa::uShort minorAxisPos = positions[4];
    const casa::uShort positionAnglePos = positions[5];

    // Extract the values from the tokens
    const casa::Quantity ra(boost::lexical_cast<casa::Double>(tokens[raPos]), deg);
    const casa::Quantity dec(boost::lexical_cast<casa::Double>(tokens[decPos]), deg);

    // Need to get rid of this hack to support the SKADS type. The plan is to drop
    // support for this filetype and just use the Duchamp format.
    casa::Quantity flux;
    if (positions[2] == 10) {
        flux = casa::Quantity(pow(10.0, boost::lexical_cast<casa::Double>(tokens[fluxPos])), Jy);
    } else {
        flux = casa::Quantity(boost::lexical_cast<casa::Double>(tokens[fluxPos]), Jy);
    }

    casa::Quantity majorAxis(boost::lexical_cast<casa::Double>(tokens[majorAxisPos]), arcsec);
    casa::Quantity minorAxis(boost::lexical_cast<casa::Double>(tokens[minorAxisPos]), arcsec);
    const casa::Quantity positionAngle(boost::lexical_cast<casa::Double>(tokens[positionAnglePos]), rad);

    // Discard if below flux limit
    if (flux.getValue(Jy) < fluxLimit.getValue(Jy)) {
        itsBelowFluxLimit++;
        return;
    }

    // Discard if outside cone
    const MVDirection searchRefDir(searchRA, searchDec);
    const MVDirection componentDir(ra, dec);
    const Quantity separation = searchRefDir.separation(componentDir, deg);
    if (separation.getValue(deg) > searchRadius.getValue(deg)) {
        itsOutsideSearchCone++;
        return;
    }

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

    // Build the Component object and add to the list
    // NOTE: The Component ID has no meaning for this accessor
    askap::cp::skymodelservice::Component c(-1, ra, dec, positionAngle,
            majorAxis, minorAxis, flux);
    list.push_back(c);
}

std::vector<casa::uShort> DuchampAccessor::getPositions(const casa::uShort nTokens)
{
    std::vector<casa::uShort> positions(6);
    if (nTokens == 17) {
        // Duchamp format
        positions[0] = 1; // RA
        positions[1] = 2; // Dec
        positions[2] = 3; // Flux
        positions[3] = 7; // Major Axis
        positions[4] = 8; // Minor Axis
        positions[5] = 9; // Position Angle
    } else if (nTokens == 13) {
        // SKADS Sky Simulations extract format
        positions[0] = 3; // RA
        positions[1] = 4; // Dec
        positions[2] = 10; // Flux
        positions[3] = 6; // Major Axis
        positions[4] = 7; // Minor Axis
        positions[5] = 5; // Position Angle
    } else {
        ASKAPTHROW(AskapError, "Malformed entry - Expected 13 or 17 tokens");
    }

    return positions;
}
