/// @file AntennaPositions.cc
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
#include "AntennaPositions.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "casa/Arrays/Matrix.h"
#include "measures/Measures/MPosition.h"
#include "measures/Measures/MCPosition.h"
#include "measures/Measures/MeasConvert.h"
#include "casa/Quanta/MVAngle.h"

// Local package includes
#include "ingestpipeline/datadef/VisChunk.h"

ASKAP_LOGGER(logger, ".AntennaPositions");

using namespace casa;
using namespace askap;
using namespace askap::cp::ingest;

//////////////////////////////////////////
// Public Methods
//////////////////////////////////////////

AntennaPositions::AntennaPositions(const LOFAR::ParameterSet& parset)
{
    // cp.ingest.uvw.antennas.names = [A0, A1, A2, A3, A4, A5]
    std::vector<std::string> antNames(parset.getStringVector("names"));
    const casa::uInt nAnt = antNames.size();
    ASKAPCHECK(nAnt > 0, "No antennas defined in parset file");

    // cp.ingest.uvw.antennas.coordinates=local
    const std::string coordsystem = parset.getString("coordinates", "local");
    ASKAPCHECK((coordsystem == "local") || (coordsystem == "global"), "Coordinates type unknown");

    // cp.ingest.uvw.antennas.scale = 1.0
    const casa::Float scale = parset.getFloat("scale", 1.0);

    /// Now we get the coordinates for each antenna in turn
    casa::Vector<double> x(nAnt);
    casa::Vector<double> y(nAnt);
    casa::Vector<double> z(nAnt);

    /// Antenna information in the form:
    /// cp.ingest.uvw.antennas.antenna0=[x,y,z]
    /// ...
    for (unsigned int iant = 0; iant < nAnt; iant++) {
        std::vector<float> xyz = parset.getFloatVector(antNames[iant]);
        x[iant] = xyz[0] * scale;
        y[iant] = xyz[1] * scale;
        z[iant] = xyz[2] * scale;
    }

    /// cp.ingest.uvw.antennas.location = [+115deg, -26deg, 192km, WGS84]
    casa::MPosition mRefLocation = asMPosition(parset.getStringVector("location"));

    // These three vectors will hold the absolute antenna locations
    Vector<double> xx(x.nelements());
    Vector<double> yy(y.nelements());
    Vector<double> zz(z.nelements());

    if (coordsystem == "global") {
        xx = x;
        yy = y;
        zz = z;
        ASKAPLOG_DEBUG_STR(logger, "Using global coordinates for the antennas");
    } else if (coordsystem == "local") {

        casa::MVAngle mvLong = mRefLocation.getAngle().getValue()(0);
        casa::MVAngle mvLat = mRefLocation.getAngle().getValue()(1);

        ASKAPLOG_DEBUG_STR(logger, "Using local coordinates for the antennas: Reference position = "
                              << mvLong.string(MVAngle::ANGLE, 7) << " "
                              << mvLat.string(MVAngle::DIG2, 7));
        local2global(xx, yy, zz, mRefLocation, x, y, z);
    } else if (coordsystem == "longlat") {
        ASKAPLOG_DEBUG_STR(logger, "Using longitude-latitude coordinates for the antennas");
        longlat2global(xx, yy, zz, mRefLocation, x, y, z);
    } else {
        ASKAPTHROW(AskapError, "Unknown coordinate system type: " << coordsystem);
    }

    itsAntXYZ.resize(3, nAnt);
    for (uInt i = 0; i < nAnt; i++) {
        itsAntXYZ(0, i) = xx(i);
        itsAntXYZ(1, i) = yy(i);
        itsAntXYZ(2, i) = zz(i);
    }
}

casa::Matrix<double> AntennaPositions::getPositionMatrix(void) const
{
    return itsAntXYZ;
}

//////////////////////////////////////////
// Private Methods
//////////////////////////////////////////

void AntennaPositions::local2global(casa::Vector<double>& xGeo, casa::Vector<double>& yGeo,
                             casa::Vector<double>& zGeo, const casa::MPosition& mRefLocation,
                             const casa::Vector<double>& xLocal, const casa::Vector<double>& yLocal,
                             const casa::Vector<double>& zLocal)
{
    casa::uInt nn = xLocal.nelements();
    xGeo.resize(nn);
    yGeo.resize(nn);
    zGeo.resize(nn);

    MPosition::Convert loc2(mRefLocation, MPosition::ITRF);
    MPosition locitrf(loc2());
    Vector<double> xyz = locitrf.get("m").getValue();

    Vector<double> ang = locitrf.getAngle("rad").getValue();
    double d1, d2;
    d1 = ang(0);
    d2 = ang(1);
    double cosLong = cos(d1);
    double sinLong = sin(d1);
    double cosLat = cos(d2);
    double sinLat = sin(d2);

    for (casa::uInt i = 0; i < nn; i++) {

        double xG1 = -sinLat * yLocal(i) + cosLat * zLocal(i);
        double yG1 = xLocal(i);

        xGeo(i) = cosLong * xG1 - sinLong * yG1 + xyz(0);
        yGeo(i) = sinLong * xG1 + cosLong * yG1 + xyz(1);

        zGeo(i) = cosLat * yLocal(i) + sinLat * zLocal(i) + xyz(2);
    }

}

void AntennaPositions::longlat2global(casa::Vector<double>& xReturned,
        casa::Vector<double>& yReturned,
        casa::Vector<double>& zReturned,
        const casa::MPosition& mRefLocation,
        const casa::Vector<double>& xIn,
        const casa::Vector<double>& yIn,
        const casa::Vector<double>& zIn)
{
    ASKAPLOG_ERROR_STR(logger, "AntennaPositions::longlat2global not yet implemented");
}

casa::MPosition AntennaPositions::asMPosition(const std::vector<std::string>& position)
{
    ASKAPCHECK(position.size()==4, "Not a valid position");

    casa::Quantity lng;
    casa::Quantity::read(lng, position[0]);
    casa::Quantity lat;
    casa::Quantity::read(lat, position[1]);
    casa::Quantity height;
    casa::Quantity::read(height, position[2]);
    casa::MPosition::Types type;
    casa::MPosition::getType(type, position[3]);
    casa::MVPosition mvPos(height, lng, lat);
    casa::MPosition pos(mvPos, type);
    return pos;
}
