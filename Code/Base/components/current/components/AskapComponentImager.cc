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

// System includes
#include <cmath>
#include <limits>
#include <algorithm>
#include <typeinfo>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "casa/aipstype.h"
#include "casa/Arrays/IPosition.h"
#include "casa/Arrays/Vector.h"
#include "casa/Quanta/MVAngle.h"
#include "casa/Quanta/MVDirection.h"
#include "casa/Quanta/MVFrequency.h"
#include "scimath/Functionals/Gaussian2D.h"
#include "images/Images/ImageInterface.h"
#include "measures/Measures/Stokes.h"
#include "measures/Measures/MDirection.h"
#include "measures/Measures/MFrequency.h"
#include "measures/Measures/MeasRef.h"
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

template <class T>
void AskapComponentImager::project(casa::ImageInterface<T>& image,
        const casa::ComponentList& list, const unsigned int term)
{
    if (list.nelements() == 0) {
        return;
    }

    const CoordinateSystem& coords = image.coordinates();
    const IPosition imageShape = image.shape();

    // Find which pixel axes correspond to the DirectionCoordinate in the
    // supplied coordinate system
    const Vector<Int> dirAxes = CoordinateUtil::findDirectionAxes(coords);
    ASKAPCHECK(dirAxes.nelements() == 2,
            "Coordinate system has unsupported number of direction axes");
    const uInt latAxis = dirAxes(0);
    const uInt longAxis = dirAxes(1);

    // Find the Direction coordinate and check the right number of axes exists
    DirectionCoordinate dirCoord = coords.directionCoordinate(
            coords.findCoordinate(Coordinate::DIRECTION));
    ASKAPCHECK(dirCoord.nPixelAxes() == 2,
            "DirectionCoordinate has unsupported number of pixel axes");
    ASKAPCHECK(dirCoord.nWorldAxes() == 2,
            "DirectionCoordinate has unsupported number of world axes");
    dirCoord.setWorldAxisUnits(Vector<String>(2, "rad"));

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
    } else {
        ASKAPLOG_DEBUG_STR(logger, "No polarisation axis, assuming Stokes I");
    }

    // Get the frequency axis and get the all the frequencies
    // as a Vector<MVFrequency>.
    const Int freqAxis = CoordinateUtil::findSpectralAxis(coords);
    ASKAPCHECK(freqAxis >= 0, "Image must have a frequency axis");
    const uInt nFreqs = static_cast<uInt>(imageShape(freqAxis));
    Vector<MVFrequency> freqValues(nFreqs);
    {
        SpectralCoordinate specCoord =
            coords.spectralCoordinate(coords.findCoordinate(Coordinate::SPECTRAL));
        specCoord.setWorldAxisUnits(Vector<String>(1, "Hz"));

        // Create Frequency MeasFrame; this will enable conversions between
        // spectral frames (e.g. the CS frame might be TOPO and the CL
        // frame LSRK)
        MFrequency::Types specConv;
        MEpoch epochConv;
        MPosition posConv;
        MDirection dirConv;
        specCoord.getReferenceConversion(specConv, epochConv, posConv, dirConv);
        for (uInt f = 0; f < nFreqs; f++) {
            Double thisFreq;
            if (!specCoord.toWorld(thisFreq, static_cast<Double>(f))) {
                ASKAPTHROW(AskapError, "Cannot convert a frequency value");
            }
            freqValues(f) = MVFrequency(thisFreq);
        }
    }

    // Process each SkyComponent individually
    for (uInt i = 0; i < list.nelements(); ++i) {
        const SkyComponent& c = list.component(i);

        for (uInt freqIdx = 0; freqIdx < nFreqs; ++freqIdx) {

            // Scale flux based on spectral model and taylor term
            const MFrequency chanFrequency(freqValues(freqIdx).get());
            Flux<Double> flux = makeFlux(c, chanFrequency, term);

            for (uInt polIdx = 0; polIdx < stokes.size(); ++polIdx) {

                switch (c.shape().type()) {
                    case ComponentType::POINT:
                        projectPointShape(image, c, latAxis, longAxis, dirCoord,
                                freqAxis, freqIdx, flux,
                                polAxis, polIdx, stokes(polIdx));
                        break;

                    case ComponentType::GAUSSIAN:
                        projectGaussianShape(image, c, latAxis, longAxis, dirCoord,
                                freqAxis, freqIdx, flux,
                                polAxis, polIdx, stokes(polIdx));
                        break;

                    default:
                        ASKAPTHROW(AskapError, "Unsupported shape type");
                        break;
                }

            } // end polIdx loop
        } // End freqIdx loop

    } // End component list loop
}

template <class T>
void AskapComponentImager::projectPointShape(casa::ImageInterface<T>& image,
        const casa::SkyComponent& c,
        const casa::Int latAxis, const casa::Int longAxis,
        const casa::DirectionCoordinate& dirCoord,
        const casa::Int freqAxis, const casa::uInt freqIdx,
        const casa::Flux<casa::Double>& flux,
        const casa::Int polAxis, const casa::uInt polIdx,
        const casa::Stokes::StokesTypes& stokes)
{
    // Convert world position to pixel position
    const MDirection& dir = c.shape().refDirection();
    Vector<Double> pixelPosition(2);
    const bool toPixelOk = dirCoord.toPixel(pixelPosition, dir);
    ASKAPCHECK(toPixelOk, "toPixel failed");

    // Don't image this component if it falls outside the image
    const IPosition imageShape = image.shape();
    const double latPosition = round(pixelPosition(0));
    const double lonPosition = round(pixelPosition(1));
    if (latPosition < 0 || latPosition > (imageShape(latAxis) - 1)
            || lonPosition < 0 || lonPosition > (imageShape(longAxis) - 1)) {
        return;
    }

    // Add to image
    const IPosition pos = makePosition(latAxis, longAxis, freqAxis, polAxis,
            static_cast<size_t>(latPosition),
            static_cast<size_t>(lonPosition),
            freqIdx, polIdx);
    image.putAt(image(pos) + (flux.copy().value(stokes, true).getValue("Jy")), pos);
}

template <class T>
void AskapComponentImager::projectGaussianShape(casa::ImageInterface<T>& image,
        const casa::SkyComponent& c,
        const casa::Int latAxis, const casa::Int longAxis,
        const casa::DirectionCoordinate& dirCoord,
        const casa::Int freqAxis, const casa::uInt freqIdx,
        const casa::Flux<casa::Double>& flux,
        const casa::Int polAxis, const casa::uInt polIdx,
        const casa::Stokes::StokesTypes& stokes)
{
    // Convert world position to pixel position
    const MDirection& dir = c.shape().refDirection();
    Vector<Double> pixelPosition(2);
    const bool toPixelOk = dirCoord.toPixel(pixelPosition, dir);
    ASKAPCHECK(toPixelOk, "toPixel failed");

    // Don't image this component if it falls outside the image
    // Note: This code will cull those components which may (due to rounding)
    // have been positioned in the edge pixels.
    const IPosition imageShape = image.shape();
    if (pixelPosition(0) < 0 || pixelPosition(0) > (imageShape(latAxis) - 1)
            || pixelPosition(1) < 0 || pixelPosition(1) > (imageShape(longAxis) - 1)) {
        return;
    }

    // Get the pixel sizes then convert the axis sizes to pixels
    const GaussianShape& cShape = dynamic_cast<const GaussianShape&>(c.shape());
    const MVAngle pixelLatSize = MVAngle(abs(dirCoord.increment()(0)));
    const MVAngle pixelLongSize = MVAngle(abs(dirCoord.increment()(1)));
    ASKAPCHECK(pixelLatSize == pixelLongSize, "Non-equal pixel sizes not supported");
    const double majorAxisPixels = cShape.majorAxisInRad() / pixelLongSize.radian();
    const double minorAxisPixels = cShape.minorAxisInRad() / pixelLongSize.radian();

    // Create the guassian function
    Gaussian2D<T> gauss;
    gauss.setXcenter(pixelPosition(0));
    gauss.setYcenter(pixelPosition(1));
    gauss.setMinorAxis(std::numeric_limits<T>::min());
    gauss.setMajorAxis(std::max(majorAxisPixels, minorAxisPixels));
    gauss.setMinorAxis(std::min(majorAxisPixels, minorAxisPixels));
    gauss.setPA(cShape.positionAngleInRad());
    gauss.setFlux(flux.copy().value(stokes, true).getValue("Jy"));

    // Determine how far to sample before the flux gets too low to be meaningful
    // We do this by going out from the centre position along both the x and y
    // axis then choose the maximum of the two
    const T epsilon = std::numeric_limits<T>::epsilon();
    const int cutoff = findCutoff(gauss, std::max(imageShape(latAxis), imageShape(longAxis)), epsilon);

    // Determine the starting and end pixels which need processing on both axes. Note
    // that these are "inclusive" ranges.
    const int startLat = std::max(0, static_cast<int>(pixelPosition(0)) - cutoff);
    const int endLat = std::min(static_cast<int>(imageShape(latAxis)),
                                static_cast<int>(pixelPosition(0)) + cutoff);
    const int startLon = std::max(0, static_cast<int>(pixelPosition(1)) - cutoff);
    const int endLon = std::min(static_cast<int>(imageShape(longAxis)),
                                static_cast<int>(pixelPosition(1)) + cutoff);

    IPosition pos = makePosition(latAxis, longAxis, freqAxis, polAxis,
            static_cast<size_t>(nearbyint(pixelPosition[0])),
            static_cast<size_t>(nearbyint(pixelPosition[1])),
            freqIdx, polIdx);

    // For each pixel in the region bounded by the source centre + cutoff
    for (int lat = startLat; lat <= endLat; ++lat) {
        for (int lon = startLon; lon <= endLon; ++lon) {
            pos(latAxis) = lat;
            pos(longAxis) = lon;
            image.putAt(image(pos) + gauss(lat, lon), pos);
        }
    }
}

IPosition AskapComponentImager::makePosition(const casa::Int latAxis, const casa::Int longAxis,
        const casa::Int spectralAxis, const casa::Int polAxis,
        const casa::uInt latIdx, const casa::uInt longIdx,
        const casa::uInt spectralIdx, const casa::uInt polIdx)
{
    // Count the number of valid axes
    uInt naxis = 0;

    if (latAxis >= 0) ++naxis;
    if (longAxis >= 0) ++naxis;
    if (spectralAxis >= 0) ++naxis;
    if (polAxis >= 0) ++naxis;

    // Create the IPosition
    IPosition pos(naxis);
    if (latAxis >= 0) pos(latAxis) = latIdx;
    if (longAxis >= 0) pos(longAxis) = longIdx;
    if (spectralAxis >= 0) pos(spectralAxis) = spectralIdx;
    if (polAxis >= 0) pos(polAxis) = polIdx;

    return pos;
}

casa::Flux<casa::Double> AskapComponentImager::makeFlux(const casa::SkyComponent& c,
        const casa::MFrequency& chanFrequency,
        const unsigned int term)
{
    // Transform flux for the given spectral model
    Flux<Double> flux;
    if (c.spectrum().type() == ComponentType::CONSTANT_SPECTRUM) {
        flux = c.flux().copy();

    } else if (c.spectrum().type() == ComponentType::SPECTRAL_INDEX) {
        // Scale flux based on spectral index
        flux = c.flux().copy();
        const Double scale = c.spectrum().sample(chanFrequency);
        flux.scaleValue(scale, scale, scale, scale);

    } else {
        ASKAPTHROW(AskapError, "Unsupported spectral model");
    }

    // Now transform flux for the given taylor term
    if (term == 0) {
        // Taylor Term 0 
        // I0 = I(v0)
    } else if (term == 1) {
        // Taylor Term 1
        // I1 = I(v0) * alpha
        Double alpha = 0.0;
        if (c.spectrum().type() == ComponentType::SPECTRAL_INDEX) {
            const casa::SpectralIndex& spectralModel =
                dynamic_cast<const casa::SpectralIndex&>(c.spectrum());
            alpha = spectralModel.index();
        }
        flux.scaleValue(alpha, alpha, alpha, alpha);
    } else if (term == 2) {
        // Taylor Term 2
        // I2 = I(v0) * (0.5 * alpha * (alpha - 1) + beta)
        Double alpha = 0.0;
        if (c.spectrum().type() == ComponentType::SPECTRAL_INDEX) {
            const casa::SpectralIndex& spectralModel =
                dynamic_cast<const casa::SpectralIndex&>(c.spectrum());
            alpha = spectralModel.index();
        }
        const Double beta = 0.0;
        const Double factor = (0.5 * alpha * (alpha - 1.0) + beta);
        flux.scaleValue(factor, factor, factor, factor);
    } else {
        ASKAPTHROW(AskapError, "Only support taylor terms 0, 1 & 2");
    }

    return flux;
}

template <class T>
int AskapComponentImager::findCutoff(const Gaussian2D<T>& gauss, const int spatialLimit,
        const double fluxLimit)
{
    int cutoff = 0;
    while (abs(gauss(gauss.xCenter() + cutoff, gauss.yCenter())) >= fluxLimit &&
           abs(gauss(gauss.xCenter(), gauss.yCenter() + cutoff)) >= fluxLimit &&
           cutoff <= spatialLimit) {
        ++cutoff;
    }
    return cutoff;
}

// Explicit instantiation
template void AskapComponentImager::project(casa::ImageInterface<float>&,
        const casa::ComponentList&, const unsigned int);
template void AskapComponentImager::project(casa::ImageInterface<double>&,
        const casa::ComponentList&, const unsigned int);
