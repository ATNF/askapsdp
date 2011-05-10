/// @file CasaWriter.cc
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
#include "cmodel/CasaWriter.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"

// Casacore includes
#include "casa/aipstype.h"
#include "casa/Quanta/MVAngle.h"
#include "casa/Quanta/Quantum.h"
#include "casa/Arrays/Matrix.h"
#include "measures/Measures/MDirection.h"
#include "components/ComponentModels/ComponentList.h"
#include "components/ComponentModels/SkyComponent.h"
#include "components/ComponentModels/GaussianShape.h"
#include "components/ComponentModels/PointShape.h"
#include "components/ComponentModels/ConstantSpectrum.h"
#include "components/ComponentModels/Flux.h"
#include "images/Images/PagedImage.h"
#include "images/Images/ComponentImager.h"
#include "casa/Arrays/IPosition.h"
#include "lattices/Lattices/TiledShape.h"
#include "coordinates/Coordinates/CoordinateSystem.h"
#include "coordinates/Coordinates/DirectionCoordinate.h"
#include "coordinates/Coordinates/SpectralCoordinate.h"

// Local package includes
#include "cmodel/ParsetUtils.h"

// Using
using namespace askap::cp::pipelinetasks;
using namespace casa;

ASKAP_LOGGER(logger, ".CasaWriter");

CasaWriter::CasaWriter(const LOFAR::ParameterSet& parset)
    : itsParset(parset.makeSubset("Cmodel."))
{
}

void CasaWriter::write(const casa::ComponentList& components)
{
    const casa::uInt nx = itsParset.getUintVector("shape").at(0);
    const casa::uInt ny = itsParset.getUintVector("shape").at(1);
    const std::string units = itsParset.getString("bunit");
    const std::string imageName = itsParset.getString("filename");

    // Open the image
    IPosition shape(3, nx, ny, 1);

    CoordinateSystem coordsys = createCoordinateSystem(nx, ny);
    PagedImage<Float> image(TiledShape(shape), coordsys, imageName);

    // Set brightness units
    image.setUnits(casa::Unit(units));

    // Build the image
    ComponentImager::project(image, components);
}

casa::CoordinateSystem CasaWriter::createCoordinateSystem(casa::uInt nx, casa::uInt ny)
{
    CoordinateSystem coordsys;
    const std::vector<std::string> dirVector = itsParset.getStringVector("direction");
    const std::vector<std::string> cellSizeVector = itsParset.getStringVector("cellsize");
    const casa::MDirection refDir = ParsetUtils::asMDirection(dirVector);

    // Direction Coordinate
    {
        Matrix<Double> xform(2, 2);
        xform = 0.0;
        xform.diagonal() = 1.0;
        const Quantum<Double> ra = ParsetUtils::asQuantity(dirVector.at(0), "deg");
        const Quantum<Double> dec = ParsetUtils::asQuantity(dirVector.at(1), "deg");
        ASKAPLOG_INFO_STR(logger, "Direction: " << ra.getValue() << " degrees, " << dec.getValue() << " degrees");

        const Quantum<Double> xcellsize = ParsetUtils::asQuantity(cellSizeVector.at(0), "arcsec") * -1.0;
        const Quantum<Double> ycellsize = ParsetUtils::asQuantity(cellSizeVector.at(1), "arcsec");
        ASKAPLOG_INFO_STR(logger, "Cellsize: " << xcellsize.getValue() << " arcsec, " << ycellsize.getValue() << " arcsec");

        casa::MDirection::Types type;
        casa::MDirection::getType(type, dirVector.at(2));
        const DirectionCoordinate radec(type, Projection(Projection::SIN),
                ra, dec, xcellsize, ycellsize, xform, nx/2, ny/2);

        coordsys.addCoordinate(radec);
    }

    // Spectral Coordinate
    {
        const Quantum<Double> f0 = ParsetUtils::asQuantity(itsParset.getString("frequency"), "Hz");
        const Quantum<Double> inc = ParsetUtils::asQuantity(itsParset.getString("increment"), "Hz");
        const Double refPix = 0.0;  // is the reference pixel
        const SpectralCoordinate sc(MFrequency::TOPO, f0, inc, refPix);

        coordsys.addCoordinate(sc);
    }  

    return coordsys;
}
