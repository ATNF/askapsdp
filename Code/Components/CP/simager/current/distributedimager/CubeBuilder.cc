/// @file CubeBuilder.cc
///
/// @copyright (c) 2013 CSIRO
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
#include <distributedimager/CubeBuilder.h>

// Include package level header file
#include <askap_simager.h>

// System includes
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

// ASKAPsoft includes
#include <askap/AskapError.h>
#include <askap/AskapLogging.h>
#include <Common/ParameterSet.h>
#include <casa/Arrays/IPosition.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <measures/Measures/Stokes.h>
#include <images/Images/PagedImage.h>
#include <casa/Quanta/Unit.h>

ASKAP_LOGGER(logger, ".CubeBuilder");

using namespace askap::cp;
using namespace casa;
using namespace std;

CubeBuilder::CubeBuilder(const LOFAR::ParameterSet& parset, const casa::uInt nchan,
                         const casa::Quantity& f0, const casa::Quantity& inc, const std::string& name)
{
    string filename = parset.getString("Images.name");

    // If necessary, replace "image" with _name_ (e.g. "psf", "weights")
    if (!name.empty()) {
        const string orig = "image";
        const size_t f = filename.find(orig);
        filename.replace(f, orig.length(), name);
    }

    // Get the image shape
    const vector<casa::uInt> imageShapeVector = parset.getUintVector("Images.shape");
    const casa::uInt nx = imageShapeVector[0];
    const casa::uInt ny = imageShapeVector[1];
    const casa::uInt npol = 1;
    const casa::IPosition cubeShape(4, nx, ny, npol, nchan);

    // Use a tile shape appropriate for plane-by-plane access
    casa::IPosition tileShape(cubeShape.nelements(), 1);
    tileShape(0) = 256;
    tileShape(1) = 256;

    const casa::CoordinateSystem csys = createCoordinateSystem(parset, nx, ny, f0, inc);

    itsCube.reset(new casa::PagedImage<float>(casa::TiledShape(cubeShape, tileShape), csys, filename));
}

CubeBuilder::~CubeBuilder()
{
}

void CubeBuilder::writeSlice(const casa::Array<float>& arr, const casa::uInt chan)
{
    casa::IPosition where(4, 0, 0, 0, chan);
    itsCube->putSlice(arr, where);
}

casa::CoordinateSystem CubeBuilder::createCoordinateSystem(const LOFAR::ParameterSet& parset,
        const casa::uInt nx, const casa::uInt ny, const casa::Quantity& f0, const casa::Quantity& inc)
{
    CoordinateSystem coordsys;
    const vector<string> dirVector = parset.getStringVector("Images.direction");
    const vector<string> cellSizeVector = parset.getStringVector("Images.cellsize");

    // Direction Coordinate
    {
        Matrix<Double> xform(2, 2);
        xform = 0.0;
        xform.diagonal() = 1.0;
        const Quantum<Double> ra = asQuantity(dirVector.at(0), "deg");
        const Quantum<Double> dec = asQuantity(dirVector.at(1), "deg");
        ASKAPLOG_DEBUG_STR(logger, "Direction: " << ra.getValue() << " degrees, "
                               << dec.getValue() << " degrees");

        const Quantum<Double> xcellsize = asQuantity(cellSizeVector.at(0), "arcsec") * -1.0;
        const Quantum<Double> ycellsize = asQuantity(cellSizeVector.at(1), "arcsec");
        ASKAPLOG_DEBUG_STR(logger, "Cellsize: " << xcellsize.getValue()
                               << " arcsec, " << ycellsize.getValue() << " arcsec");

        casa::MDirection::Types type;
        casa::MDirection::getType(type, dirVector.at(2));
        const DirectionCoordinate radec(type, Projection(Projection::SIN),
                                        ra, dec, xcellsize, ycellsize, xform, nx / 2, ny / 2);

        coordsys.addCoordinate(radec);
    }

    // Stokes Coordinate
    {
        Vector<Int> stokes;
        //if (parset.isDefined("stokes")) {
        //    stokes = parseStokes(parset.getStringVector("stokes"));
        //} else {
        stokes.resize(1);
        stokes(0) = Stokes::I;
        //}

        const StokesCoordinate stokescoord(stokes);
        coordsys.addCoordinate(stokescoord);
    }
    // Spectral Coordinate
    {
        const Double refPix = 0.0;  // is the reference pixel
        const SpectralCoordinate sc(MFrequency::TOPO, f0, inc, refPix);

        coordsys.addCoordinate(sc);
    }

    return coordsys;
}
