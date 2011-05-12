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
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cmath>

// ASKAPsoft include
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "boost/lexical_cast.hpp"

// Casacore includes
#include "casa/aipstype.h"
#include "casa/Quanta/Quantum.h"
#include "casa/Quanta/MVDirection.h"
#include "measures/Measures/MDirection.h"
#include "components/ComponentModels/ComponentList.h"
#include "components/ComponentModels/SkyComponent.h"
#include "components/ComponentModels/ComponentList.h"
#include "components/ComponentModels/GaussianShape.h"
#include "components/ComponentModels/PointShape.h"
#include "components/ComponentModels/ConstantSpectrum.h"
#include "components/ComponentModels/Flux.h"

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

// TODO: This doesn actually implement a conesearch. Instead it returns
// the full result set.
casa::ComponentList DuchampAccessor::coneSearch(const casa::Quantity& ra,
        const casa::Quantity& dec,
        const casa::Quantity& searchRadius,
        const casa::Quantity& fluxLimit)
{
    ASKAPLOG_DEBUG_STR(logger, "Cone search - ra: " << ra.getValue("deg")
                           << " deg, dec: " << dec.getValue("deg")
                           << " deg, radius: " << searchRadius.getValue("deg")
                           << " deg, Fluxlimit: " << fluxLimit.getValue("Jy") << " Jy");

    // Seek back to the beginning of the buffer before reading line by line
    itsFile->seekg(0, std::ios::beg);
    std::string line;
    itsBelowFluxLimit = 0;
    itsOutsideSearchCone = 0;
    casa::uLong total = 0;
    ComponentList list;

    while (getline(*itsFile, line)) {
        if (line.find_first_of("#") == std::string::npos) {
            processLine(line, ra, dec, searchRadius, fluxLimit, list);
            total++;

            if (total % 100000 == 0) {
                ASKAPLOG_DEBUG_STR(logger, "Read " << total << " component entries");
            }
        }
    }

    ASKAPLOG_DEBUG_STR(logger, "Sources discarded due to threshold: " << itsBelowFluxLimit);
    ASKAPLOG_DEBUG_STR(logger, "Sourced discarded due to being outside the search cone: " << itsOutsideSearchCone);
    return list;
}

void DuchampAccessor::processLine(const std::string& line,
                                  const casa::Quantity& searchRA,
                                  const casa::Quantity& searchDec,
                                  const casa::Quantity& searchRadius,
                                  const casa::Quantity& fluxLimit,
                                  casa::ComponentList& list)
{
    // Positions of the tokens of interest

    //////////////////////////////////////////////////////////////////////////////////////
    // The below is the Duchamp ASCII format
    //////////////////////////////////////////////////////////////////////////////////////
    /*
       const casa::Short totalTokens = 17;
       const casa::Short raPos = 1;
       const casa::Short decPos = 2;
       const casa::Short fluxPos = 3;
       const casa::Short majorAxisPos = 7;
       const casa::Short minorAxisPos = 8;
       const casa::Short positionAnglePos = 9;
       */

    //////////////////////////////////////////////////////////////////////////////////////
    // The below Matt's SKADS .dat file format
    //////////////////////////////////////////////////////////////////////////////////////
    const casa::uShort totalTokens = 13;
    const casa::uShort raPos = 3;
    const casa::uShort decPos = 4;
    const casa::uShort fluxPos = 10;
    const casa::uShort majorAxisPos = 6;
    const casa::uShort minorAxisPos = 7;
    const casa::uShort positionAnglePos = 5;

    // Tokenize the line
    stringstream iss(line);
    vector<string> tokens;
    copy(istream_iterator<string>(iss),
         istream_iterator<string>(),
         back_inserter<vector<string> >(tokens));

    if (tokens.size() != totalTokens) {
        ASKAPTHROW(AskapError, "Malformed entry - Expected " << totalTokens << " tokens");
    }

    // Extract the values from the tokens
    const casa::Double _ra = boost::lexical_cast<casa::Double>(tokens[raPos]);
    const casa::Double _dec = boost::lexical_cast<casa::Double>(tokens[decPos]);
    const casa::Double _flux = pow(10.0, boost::lexical_cast<casa::Double>(tokens[fluxPos]));
    casa::Double _majorAxis = boost::lexical_cast<casa::Double>(tokens[majorAxisPos]);
    casa::Double _minorAxis = boost::lexical_cast<casa::Double>(tokens[minorAxisPos]);
    const casa::Double _positionAngle = boost::lexical_cast<casa::Double>(tokens[positionAnglePos]);

    // Discard if below flux limit
    if (_flux < fluxLimit.getValue("Jy")) {
        itsBelowFluxLimit++;
        return;
    }

    // Discard if outside cone
    const Quantity ra(_ra, "deg");
    const Quantity dec(_dec, "deg");
    const MVDirection searchRefDir(searchRA, searchDec);
    const MVDirection componentDir(ra, dec);
    const Quantity separation = searchRefDir.separation(componentDir, "deg");

    if (separation.getValue("deg") > searchRadius.getValue("deg")) {
        itsOutsideSearchCone++;
        return;
    }

    // Build either a GaussianShape or PointShape
    const MDirection dir(ra, dec, MDirection::J2000);
    const Flux<casa::Double> flux(_flux, 0.0, 0.0, 0.0);
    const ConstantSpectrum spectrum;

    if (_majorAxis > 0.0 || _minorAxis > 0.0) {

        // Ensure major axis is larger than minor axis
        if (_majorAxis < _minorAxis) {
            casa::Double tmp = _minorAxis;
            _minorAxis = _majorAxis;
            _majorAxis = tmp;
        }

        ASKAPDEBUGASSERT(_majorAxis >= _minorAxis);

        // Fix for casa::Gaussian2D not liking minor axis of 0.0
        if (_minorAxis == 0.0) {
            _minorAxis = 1.0e-15;
        }

        const GaussianShape shape(dir,
                                  Quantity(_majorAxis, "arcsec"),
                                  Quantity(_minorAxis, "arcsec"),
                                  Quantity(_positionAngle, "rad"));

        list.add(SkyComponent(flux, shape, spectrum));
    } else {
        const PointShape shape(dir);
        list.add(SkyComponent(flux, shape, spectrum));
    }
}
