/// @file ImageFactory.cc
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
#include "cmodel/ImageFactory.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <string>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "Common/ParameterSet.h"

// Casacore includes
#include "casa/aipstype.h"
#include "casa/Quanta.h"
#include "casa/Quanta/Quantum.h"
#include "casa/Arrays/Matrix.h"
#include "measures/Measures/MDirection.h"
#include "casa/Arrays/IPosition.h"
#include "lattices/Lattices/TiledShape.h"
#include "coordinates/Coordinates/CoordinateSystem.h"
#include "coordinates/Coordinates/DirectionCoordinate.h"
#include "coordinates/Coordinates/SpectralCoordinate.h"

// Using
using namespace askap;
using namespace askap::cp::pipelinetasks;
using namespace casa;

ASKAP_LOGGER(logger, ".ImageFactory");

casa::TempImage<casa::Float> ImageFactory::createTempImage(const LOFAR::ParameterSet& parset)
{
    const casa::uInt nx = parset.getUintVector("shape").at(0);
    const casa::uInt ny = parset.getUintVector("shape").at(1);
    const std::string units = parset.getString("bunit");

    // Open the image
    IPosition shape(3, nx, ny, 1);

    CoordinateSystem coordsys = createCoordinateSystem(nx, ny, parset);
    casa::TempImage<casa::Float> image(TiledShape(shape), coordsys);
    image.set(0.0);

    // Set brightness units
    image.setUnits(casa::Unit(units));
    return image;
}

casa::PagedImage<casa::Float> ImageFactory::createPagedImage(const LOFAR::ParameterSet& parset,
        const std::string& filename)
{
    const casa::uInt nx = parset.getUintVector("shape").at(0);
    const casa::uInt ny = parset.getUintVector("shape").at(1);
    const std::string units = parset.getString("bunit");

    // Open the image
    IPosition shape(3, nx, ny, 1);

    CoordinateSystem coordsys = createCoordinateSystem(nx, ny, parset);
    casa::PagedImage<casa::Float> image(TiledShape(shape), coordsys, filename);
    image.set(0.0);

    // Set brightness units
    image.setUnits(casa::Unit(units));
    return image;
}

casa::CoordinateSystem ImageFactory::createCoordinateSystem(casa::uInt nx, casa::uInt ny,
        const LOFAR::ParameterSet& parset)
{
    CoordinateSystem coordsys;
    const std::vector<std::string> dirVector = parset.getStringVector("direction");
    const std::vector<std::string> cellSizeVector = parset.getStringVector("cellsize");

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

    // Spectral Coordinate
    {
        const Quantum<Double> f0 = asQuantity(parset.getString("frequency"), "Hz");
        const Quantum<Double> inc = asQuantity(parset.getString("increment"), "Hz");
        const Double refPix = 0.0;  // is the reference pixel
        const SpectralCoordinate sc(MFrequency::TOPO, f0, inc, refPix);

        coordsys.addCoordinate(sc);
    }

    return coordsys;
}
