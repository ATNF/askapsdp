/// @file AskapComponentImager.cc
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
/// This implementation is based on the Casacore Component Imager.

// Include own header file first
#include "AskapComponentImager.h"

// Include package level header file
#include "askap_components.h"

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "casa/Arrays/Vector.h"
#include "casa/Quanta/MVAngle.h"
#include "casa/Quanta/MVDirection.h"
#include "casa/Quanta/MVFrequency.h"
#include "measures/Measures/Stokes.h"
#include "measures/Measures/MDirection.h"
#include "measures/Measures/MFrequency.h"
#include "measures/Measures/MeasRef.h"
#include "images/Images/ImageInterface.h"
#include "components/ComponentModels/SkyComponent.h"
#include "components/ComponentModels/ComponentList.h"
#include "components/ComponentModels/Flux.h"
#include "components/ComponentModels/ConstantSpectrum.h"
#include "components/ComponentModels/SpectralIndex.h"
#include "components/ComponentModels/SpectralModel.h"
#include "components/ComponentModels/ComponentType.h"
#include "components/ComponentModels/ComponentShape.h"
#include "components/ComponentModels/PointShape.h"
#include "components/ComponentModels/GaussianShape.h"
#include "components/ComponentModels/DiskShape.h"
#include "coordinates/Coordinates/CoordinateUtil.h"
#include "coordinates/Coordinates/CoordinateSystem.h"
#include "coordinates/Coordinates/DirectionCoordinate.h"
#include "coordinates/Coordinates/SpectralCoordinate.h"

ASKAP_LOGGER(logger, ".AskapComponentImager");

using namespace askap;
using namespace askap::components;
using namespace casa;

template<class T>
void AskapComponentImager::project(casa::ImageInterface<T>& image, const casa::ComponentList& list)
{
    if (list.nelements() == 0) {
        return;
    }

    const CoordinateSystem& coords = image.coordinates();
    const IPosition imageShape = image.shape();

    // Find which pixel axes correspond to the DirectionCoordinate in the
    // supplied coordinate system
    const Vector<Int> dirAxes = CoordinateUtil::findDirectionAxes(coords);
    ASKAPCHECK(dirAxes.nelements() == 2, "Coordinate system has unsupported number of direction axes");
    const uInt latAxis = dirAxes(0);
    const uInt longAxis = dirAxes(1);

    ASKAPLOG_INFO_STR(logger, "latAxis: " << latAxis); // DEBUG
    ASKAPLOG_INFO_STR(logger, "longAxis: " << longAxis); // DEBUG

    // Find the Direction coordinate and check the right number of axes exists
    DirectionCoordinate dirCoord = coords.directionCoordinate(coords.findCoordinate(Coordinate::DIRECTION));
    ASKAPCHECK(dirCoord.nPixelAxes() == 2, "DirectionCoordinate has unsupported number of pixel axes");
    ASKAPCHECK(dirCoord.nWorldAxes() == 2, "DirectionCoordinate has unsupported number of world axes");
    dirCoord.setWorldAxisUnits(Vector<String>(2, "rad"));

    // Get the pixel sizes
    MVAngle pixelLatSize, pixelLongSize;
    {
        const Vector<Double> inc = dirCoord.increment();
        pixelLatSize = MVAngle(abs(inc(0)));
        pixelLongSize = MVAngle(abs(inc(1)));
    }

    ASKAPLOG_INFO_STR(logger, "pixelLatSize: " << pixelLatSize); // DEBUG
    ASKAPLOG_INFO_STR(logger, "pixelLongSize: " << pixelLongSize); // DEBUG

    ///// DEBUG
    dirCoord.setWorldAxisUnits(Vector<String>(2, "deg"));
    const Vector<Double>& refPix = dirCoord.referencePixel();
    Vector<Double> pixel(2), world(2);
    ASKAPLOG_INFO_STR(logger, "refPix.size(): " << refPix.size());
    ASKAPLOG_INFO_STR(logger, "refPix[0]: " << refPix[0]);
    ASKAPLOG_INFO_STR(logger, "refPix[1]: " << refPix[1]);

    dirCoord.toWorld(world, pixel);
    ASKAPLOG_INFO_STR(logger, "world[0]: " << world[0]);
    ASKAPLOG_INFO_STR(logger, "world[1]: " << world[1]);

    //world[0] = 187.5;
    //world[1] = -45.0;
    const bool res = dirCoord.toPixel(pixel, world);
    ASKAPLOG_INFO_STR(logger, "toPixel(): " << res);
    ASKAPLOG_INFO_STR(logger, "pixel[0]: " << pixel[0]);
    ASKAPLOG_INFO_STR(logger, "pixel[1]: " << pixel[1]);


    dirCoord.setWorldAxisUnits(Vector<String>(2, "rad"));
    ///// DEBUG END

    // Check if there is a Stokes Axes and if so which polarizations.
    // Otherwise only image the I polarisation.
    Vector<Stokes::StokesTypes> stokes;
    // Vector stating which polarisations are on each plane
    // Find which axis is the stokes pixel axis
    const Int polAxis = CoordinateUtil::findStokesAxis(stokes, coords);
    const uInt nStokes = stokes.nelements();
    if (polAxis >= 0) {
        ASKAPASSERT(static_cast<uInt>(imageShape(polAxis)) == nStokes);
        // If there is a Stokes axis it can only contain Stokes::I,Q,U,V pols.
        for (uInt p = 0; p < nStokes; ++p) {
            ASKAPCHECK(stokes(p) == Stokes::I || stokes(p) == Stokes::Q ||
                    stokes(p) == Stokes::U || stokes(p) == Stokes::V,
                    "Stokes axis can only contain I, Q, U or V pols");
        }
    }
    ASKAPLOG_INFO_STR(logger, "polAxis: " << polAxis); // DEBUG

    // Get the frequency axis and get the all the frequencies
    // as a Vector<MVFrequency>.
    MeasRef<MFrequency> freqRef;
    const Int freqAxis = CoordinateUtil::findSpectralAxis(coords);
    ASKAPCHECK(freqAxis >= 0, "Image must have a frequency axis");
    const uInt nFreqs = static_cast<uInt>(imageShape(freqAxis));
    Vector<MVFrequency> freqValues(nFreqs);
    SpectralCoordinate specCoord =
        coords.spectralCoordinate(coords.findCoordinate(Coordinate::SPECTRAL));
    specCoord.setWorldAxisUnits(Vector<String>(1, "Hz"));

    ASKAPLOG_INFO_STR(logger, "freqAxis: " << freqAxis); // DEBUG

    // Process each SkyComponent individually
    for (uInt i = 0; i < list.nelements(); ++i) {
        const SkyComponent& c = list.component(i);
        for (uInt freq = 0; freq < nFreqs; ++freq) {
            // Handle point shapes only right now
            if (c.shape().type() == ComponentType::POINT) {
                imagePointShape(image, c, freq, dirCoord);
            }
        }
    }
}

template<class T>
void AskapComponentImager::imagePointShape(casa::ImageInterface<T>& image,
        const casa::SkyComponent& c, const casa::uInt freq,
        const casa::DirectionCoordinate& dirCoord)
{
                // Convert world position to pixel position
                const MDirection& dir = c.shape().refDirection();
                Vector<Double> pixelPosition(2);
                const bool toPixelOk = dirCoord.toPixel(pixelPosition, dir);
                ASKAPCHECK(toPixelOk, "toPixel failed");
                IPosition pos(3, pixelPosition[0], pixelPosition[1], freq);
                T oldVal = image.getAt(pos);
                ASKAPLOG_INFO_STR(logger, "Adding component with flux: " << abs(c.flux().value(0)));
                image.putAt(oldVal + abs(c.flux().value(0)), pos);
}

// Explicit instantiation
template void AskapComponentImager::project(casa::ImageInterface<float>&, const casa::ComponentList&);
template void AskapComponentImager::project(casa::ImageInterface<double>&, const casa::ComponentList&);
