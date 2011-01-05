/// @file ParsetConfiguration.cc
///
/// @copyright (c) 2010 CSIRO
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
#include "ParsetConfiguration.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <string>
#include <sstream>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Vector.h"
#include "measures/Measures/MDirection.h"

// Local package includes
#include "ingestutils/AntennaPositions.h"

ASKAP_LOGGER(logger, ".ParsetConfiguration");

using namespace casa;
using namespace askap;
using namespace askap::cp::ingest;

//////////////////////////////////////////
// Public Methods
//////////////////////////////////////////

ParsetConfiguration::ParsetConfiguration(const LOFAR::ParameterSet& parset) :
        itsParset(parset)
{
}

void ParsetConfiguration::getAntennas(casa::Vector<std::string>& names,
                                      std::string& station,
                                      casa::Matrix<double>& antXYZ,
                                      casa::Matrix<double>& offset,
                                      casa::Vector<casa::Double>& dishDiameter,
                                      casa::Vector<std::string>& mount)
{
    const LOFAR::ParameterSet antSubset(itsParset.makeSubset("antennas."));

    // Get station name
    station = antSubset.getString("station", "");

    // Get antenna names and number
    names = antSubset.getStringVector("names");
    const casa::uInt nAnt = names.size();
    ASKAPCHECK(nAnt > 0, "No antennas defined in parset file");

    // Get antenna positions
    AntennaPositions antPos(antSubset);
    antXYZ = antPos.getPositionMatrix();

    // Get antenna diameter
    dishDiameter.resize(nAnt);
    dishDiameter = asQuantity(antSubset.getString("diameter",
                              "12m")).getValue("m");
    ASKAPCHECK(dishDiameter(0) > 0.0, "Antenna diameter not positive");

    // Get mount type
    mount.resize(nAnt);
    mount = antSubset.getString("mount", "equatorial");
    ASKAPCHECK((mount(0) == "equatorial") || (mount(0) == "alt-az"),
               "Antenna mount type unknown");

    // Offset not supported
    offset.resize(antXYZ.shape());
    offset.set(0.0);
}

void ParsetConfiguration::getFeeds(casa::String& mode,
                                   casa::Vector<double>& x,
                                   casa::Vector<double>& y,
                                   casa::Vector<casa::String>& pol)
{
    std::vector<string> feedNames(itsParset.getStringVector("feeds.names"));
    const int nFeeds = feedNames.size();
    ASKAPCHECK(nFeeds > 0, "No feeds specified");

    x.resize(nFeeds);
    y.resize(nFeeds);
    pol.resize(nFeeds);

    mode = itsParset.getString("feeds.mode", "perfect X Y");

    for (int feed = 0; feed < nFeeds; feed++) {
        ostringstream os;
        os << "feeds." << feedNames[feed];
        std::vector<double> xy(itsParset.getDoubleVector(os.str()));
        x[feed] = xy[0];
        y[feed] = xy[1];
        pol[feed] = "X Y";
    }

    if (itsParset.isDefined("feeds.spacing")) {
        const casa::Quantity qspacing = asQuantity(itsParset.getString("feeds.spacing"));
        const double spacing = qspacing.getValue("rad");
        ASKAPLOG_DEBUG_STR(logger, "Scaling feed specifications by " << qspacing);
        x *= spacing;
        y *= spacing;
    }
}

void ParsetConfiguration::getSpWindows(casa::String& spWindowName, int& nChan,
        casa::Quantity& startFreq, casa::Quantity& freqInc,
        casa::String& stokesString)
{
    spWindowName = itsParset.getString("spw.name");
    nChan = itsParset.getUint32("spw.nchan");
    startFreq = asQuantity(itsParset.getString("spw.start_freq"));
    freqInc = asQuantity(itsParset.getString("spw.freq_inc"));
    stokesString = itsParset.getString("spw.stokes");
}

void ParsetConfiguration::getFields(casa::String& fieldName,
        casa::MDirection& fieldDirection,
        casa::String& calCode)
{
    fieldName = itsParset.getString("field.name");
    fieldDirection = asMDirection(itsParset.getStringVector("field.direction"));
    calCode = "";
}


//////////////////////////////////////////
// Private Methods
//////////////////////////////////////////

casa::Quantity ParsetConfiguration::asQuantity(const std::string& str)
{
    casa::Quantity q;
    casa::Quantity::read(q, str);
    return q;
}

casa::MDirection ParsetConfiguration::asMDirection(const std::vector<std::string>& direction)
{
    ASKAPCHECK(direction.size()==3, "Not a valid direction");

    casa::Quantity lng;
    casa::Quantity::read(lng, direction[0]);
    casa::Quantity lat;
    casa::Quantity::read(lat, direction[1]);
    casa::MDirection::Types type;
    casa::MDirection::getType(type, direction[2]);
    casa::MDirection dir(lng, lat, type);
    return dir;
}

