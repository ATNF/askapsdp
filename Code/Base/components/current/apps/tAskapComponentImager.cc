/// @file tAskapComponentImager.cc
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

// Include package level header file
#include "askap_components.h"

// System include
#include <fstream>
#include <sstream>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/Log4cxxLogSink.h"
#include "casa/aipstype.h"
#include "casa/Arrays/IPosition.h"
#include "casa/Quanta.h"
#include "casa/Quanta/Quantum.h"
#include "images/Images/PagedImage.h"
#include "measures/Measures/MDirection.h"
#include "lattices/Lattices/TiledShape.h"
#include "components/ComponentModels/SkyComponent.h"
#include "components/ComponentModels/ComponentList.h"
#include "components/ComponentModels/Flux.h"
#include "components/ComponentModels/ConstantSpectrum.h"
#include "components/ComponentModels/PointShape.h"
#include "coordinates/Coordinates/CoordinateSystem.h"
#include "coordinates/Coordinates/DirectionCoordinate.h"
#include "coordinates/Coordinates/SpectralCoordinate.h"

// Local packages includes
#include "components/AskapComponentImager.h"

// Using
using namespace askap;
using namespace askap::components;
using namespace casa;
using namespace std;

casa::CoordinateSystem createCoordinateSystem(casa::uInt nx, casa::uInt ny)
{
    CoordinateSystem coordsys;

    // Direction Coordinate
    {
        Matrix<Double> xform(2, 2);
        xform = 0.0;
        xform.diagonal() = 1.0;
        const Quantum<Double> ra(187.5, "deg");
        const Quantum<Double> dec(-45.0, "deg");

        const Quantum<Double> xcellsize(5.0 * -1.0, "arcsec");
        const Quantum<Double> ycellsize(5.0, "arcsec");

        const DirectionCoordinate radec(MDirection::J2000, Projection(Projection::SIN),
                                        ra, dec, xcellsize, ycellsize, xform, nx / 2, ny / 2);

        coordsys.addCoordinate(radec);
    }

    // Spectral Coordinate
    {
        const Quantum<Double> f0(1.4, "MHz");
        const Quantum<Double> inc(300.0, "MHz");
        const Double refPix = 0.0;  // is the reference pixel
        const SpectralCoordinate sc(MFrequency::TOPO, f0, inc, refPix);

        coordsys.addCoordinate(sc);
    }

    return coordsys;
}

// main()
int main(int argc, char *argv[])
{
    // Initialize the logger before we use it. If a log configuraton
    // exists in the current directory then use it, otherwise try to
    // use the programs default one.
    std::ifstream config("askap.log_cfg", std::ifstream::in);

    if (config) {
        ASKAPLOG_INIT("askap.log_cfg");
    } else {
        std::ostringstream ss;
        ss << argv[0] << ".log_cfg";
        ASKAPLOG_INIT(ss.str().c_str());
    }

    // Ensure that CASA log messages are captured
    casa::LogSinkInterface* globalSink = new Log4cxxLogSink();
    casa::LogSink::globalSink(globalSink);

    // Centre of the image
    const MDirection dir(casa::Quantity(187.5, "deg"),
            casa::Quantity(-45.05, "deg"),
            MDirection::J2000);

    // Create a component
    const Flux<casa::Double> flux(1.0); // Q=U=V=0
    const ConstantSpectrum spectrum;
    const PointShape shape(dir);

    // Add it to the component list
    ComponentList list;
    list.add(SkyComponent(flux, shape, spectrum));

    // Open the image
    const uInt nx = 256;
    const uInt ny = nx;
    IPosition imgShape(3, nx, ny, 1);

    CoordinateSystem coordsys = createCoordinateSystem(nx, ny);
    casa::PagedImage<casa::Float> image(TiledShape(imgShape), coordsys, "image.tAskapComponentImager");

    // Set brightness units
    image.setUnits(casa::Unit("Jy/pixel"));

    AskapComponentImager::project(image, list);

    return 0;
}
