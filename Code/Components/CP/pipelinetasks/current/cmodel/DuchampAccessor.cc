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
    : itsFile(filename.c_str())
{
}

DuchampAccessor::~DuchampAccessor()
{
    itsFile.close();
}

// TODO: This doesn actually implement a conesearch. Instead it returns
// the full result set.
casa::ComponentList DuchampAccessor::coneSearch(const casa::Quantity& ra,
        const casa::Quantity& dec,
        const casa::Quantity& searchRadius,
        const casa::Quantity& fluxLimit)
{
    ComponentList list;

    ASKAPLOG_DEBUG_STR(logger, "Cone search - RA: " << ra.getValue("deg") << ", DEC: " << dec.getValue("deg") 
            << ", RADIUS: " << searchRadius.getValue("deg") << ", Fluxlimit: " << fluxLimit.getValue("deg"));

    // Seek back to the beginning of the buffer before reading line by line
    itsFile.seekg(0, std::ios::beg);
    std::string line;
    casa::uLong belowFluxLimit = 0;
    casa::uLong outsideSearchCone = 0;
    casa::uLong total = 0;
    while (getline(itsFile, line)) {
        if (line.find_first_of("#") == std::string::npos) {
            SkyComponent sc = createComponent(line);
            total++;
            if (total % 50000 == 0) {
                ASKAPLOG_DEBUG_STR(logger, "Read " << total << " component entries");
            }
            ASKAPCHECK(sc.ok(), "SkyComponent is not correct/consistent");

            // Discard if below flux limit
            if (sc.flux().value(0) < fluxLimit.getValue("Jy")) {
                belowFluxLimit++;
                continue;
            }

            // Create a MDirection for the search centre to deal with normalising to +/-180
            const casa::MDirection searchRefDir(ra, dec, MDirection::J2000);
            const casa::Quantity _ra = searchRefDir.getValue().getLong("deg");
            const casa::Quantity _dec = searchRefDir.getValue().getLat("deg");

            // Discard if position falls outside the search cone
            //
            // TODO: The below uses simple (Euclidean geometry) Pythagoras
            // theorem. Need to handle spherical geometry, wrap-around etc.
            const casa::MDirection componentDir = sc.shape().refDirection();
            const casa::Double componentRA = componentDir.getValue().getLong("deg").getValue("deg");
            const casa::Double componentDec = componentDir.getValue().getLat("deg").getValue("deg");
        

            const casa::Double a = abs(_dec.getValue("deg")) - abs(componentDec);
            const casa::Double b = abs(_ra.getValue("deg")) - abs(componentRA);
            const casa::Double c = sqrt((pow(a, 2) + pow(b, 2)));
            if (c > searchRadius.getValue("deg")) {
//                ASKAPLOG_DEBUG_STR(logger, "Outside search radius! Dist: " << c << ", search radius: " << searchRadius.getValue("deg"));
                outsideSearchCone++;
                continue;
            }
//            ASKAPLOG_DEBUG_STR(logger, "Dist: " << c << ", search radius: " << searchRadius.getValue("deg"));

            list.add(sc);
        }
    }

    ASKAPLOG_DEBUG_STR(logger, "Sources falling below the flux threshold: " << belowFluxLimit);
    ASKAPLOG_DEBUG_STR(logger, "Sources falling outside the search cone: " << outsideSearchCone);
    return list;
}

SkyComponent DuchampAccessor::createComponent(const std::string& line)
{
    // Tokenize the line
    stringstream iss(line); 
    vector<string> tokens;
    copy(istream_iterator<string>(iss),
            istream_iterator<string>(),
            back_inserter<vector<string> >(tokens));
/*
    if (tokens.size() != 17) {
        std::cerr << "nTokens: " << tokens.size() << std::endl;
        ASKAPTHROW(AskapError, "Malformed entry");
    }

    // Build either a GaussianShape or PointShape
    const casa::Double _ra = boost::lexical_cast<casa::Double>(tokens[1]);
    const casa::Double _dec = boost::lexical_cast<casa::Double>(tokens[2]);
    const casa::Double _flux = boost::lexical_cast<casa::Double>(tokens[3]);
    casa::Double _majorAxis = boost::lexical_cast<casa::Double>(tokens[7]);
    casa::Double _minorAxis = boost::lexical_cast<casa::Double>(tokens[8]);
    const casa::Double _positionAngle = boost::lexical_cast<casa::Double>(tokens[9]);
*/   
 
    //////////////////////////////////////////////////////////////////////////////////////
    // The below Matt's SKADS .dat file format
    //////////////////////////////////////////////////////////////////////////////////////
    if (tokens.size() != 13) {
        std::cerr << "nTokens: " << tokens.size() << std::endl;
        ASKAPTHROW(AskapError, "Malformed entry");
    }

    // Build either a GaussianShape or PointShape
    const casa::Double _ra = boost::lexical_cast<casa::Double>(tokens[3]);
    const casa::Double _dec = boost::lexical_cast<casa::Double>(tokens[4]);
    const casa::Double _flux = pow(10, boost::lexical_cast<casa::Double>(tokens[10]));
    casa::Double _majorAxis = boost::lexical_cast<casa::Double>(tokens[6]);
    casa::Double _minorAxis = boost::lexical_cast<casa::Double>(tokens[7]);
    const casa::Double _positionAngle = boost::lexical_cast<casa::Double>(tokens[5]);
    //////////////////////////////////////////////////////////////////////////////////////

    //ASKAPLOG_DEBUG_STR(logger, "Creating component: " << _majorAxis << ", " << _minorAxis << " Component: " <<
    //        tokens[0]);

    const Quantum<casa::Double> ra(_ra, "deg");
    const Quantum<casa::Double> dec(_dec, "deg");
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
                Quantity(_positionAngle, "deg"));
        return SkyComponent(flux, shape, spectrum);
    } else {
        const PointShape shape(dir);
        return SkyComponent(flux, shape, spectrum);
    }
}
