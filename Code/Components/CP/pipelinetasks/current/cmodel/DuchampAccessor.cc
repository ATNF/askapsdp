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

DuchampAccessor::DuchampAccessor(std::ifstream& file)
    : itsFile(file)
{
}

// TODO: This doesn actually implement a conesearch. Instead it returns
// the full result set.
casa::ComponentList DuchampAccessor::coneSearch(const casa::Double ra,
        const casa::Double dec,
        const casa::Double searchRadius)
{
    ComponentList list;

    // Seek back to the beginning of the buffer before reading line by line
    itsFile.seekg(0, std::ios::beg);
    std::string line;
    while (getline(itsFile, line)) {
        if (line.find_first_of("#") == std::string::npos) {
            SkyComponent c = createComponent(line);
            ASKAPCHECK(c.ok(), "SkyComponent is not correct/consistent");
            list.add(c);
        }
    }

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

    if (tokens.size() != 17) {
        std::cerr << "nTokens: " << tokens.size() << std::endl;
        ASKAPTHROW(AskapError, "Malformed entry");
    }

    // Build either a GaussianShape or PointShape
    const casa::Double _ra = boost::lexical_cast<casa::Double>(tokens[1]);
    const casa::Double _dec = boost::lexical_cast<casa::Double>(tokens[2]);
    const casa::Double _flux = boost::lexical_cast<casa::Double>(tokens[3]);
    const casa::Double _majorAxis = boost::lexical_cast<casa::Double>(tokens[7]);
    const casa::Double _minorAxis = boost::lexical_cast<casa::Double>(tokens[8]);
    const casa::Double _positionAngle = boost::lexical_cast<casa::Double>(tokens[9]);

    const Quantum<casa::Double> ra(_ra, "deg");
    const Quantum<casa::Double> dec(_dec, "deg");
    const MDirection dir(ra, dec, MDirection::J2000);
    const Flux<casa::Double> flux(_flux, 0.0, 0.0, 0.0);
    const ConstantSpectrum spectrum;

    if (_majorAxis > 0.0 || _minorAxis > 0.0) {
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
