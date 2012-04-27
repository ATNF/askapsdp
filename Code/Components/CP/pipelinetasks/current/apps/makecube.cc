/// @file makecube.cc
///
/// @copyright (c) 2009 CSIRO
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
#include "askap_pipelinetasks.h"

// System includes
#include <cstdlib>
#include <iostream>
#include <iomanip>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "askap/Log4cxxLogSink.h"
#include "askap/StatReporter.h"
#include "casa/Logging/LogIO.h"
#include "casa/Logging/LogSinkInterface.h"
#include "casa/Arrays/Array.h"
#include "casa/Quanta/Unit.h"
#include "coordinates/Coordinates/CoordinateSystem.h"
#include "coordinates/Coordinates/SpectralCoordinate.h"
#include "images/Images/PagedImage.h"

using namespace askap;

ASKAP_LOGGER(logger, ".makecube");

// Creates the filename given the prefix and channel number
std::string getName(const std::string& base, int chan)
{
    std::stringstream name;
    name << base << chan;
    return name.str();
}

void assertValidCoordinates(const casa::CoordinateSystem& csys)
{
    const int whichSpectral = csys.findCoordinate(casa::Coordinate::SPECTRAL);
    ASKAPCHECK(whichSpectral > -1,
            "No spectral coordinate present in the coordinate system of the first image.");

    const casa::Vector<casa::Int> axesSpectral = csys.pixelAxes(whichSpectral);
    ASKAPCHECK(axesSpectral.nelements() == 1, "Spectral axis "<< whichSpectral
            << " is expected to correspond to just one pixel axes, you have "
            << axesSpectral);
}

double getChanFreq(const casa::CoordinateSystem& csys)
{
    assertValidCoordinates(csys);
    const int whichSpectral = csys.findCoordinate(casa::Coordinate::SPECTRAL);
    const casa::Vector<casa::Int> axesSpectral = csys.pixelAxes(whichSpectral);

    casa::SpectralCoordinate freq(csys.spectralCoordinate(whichSpectral));
    double chanFreq;
    freq.toWorld(chanFreq, 0.0);
    return chanFreq;
}

double getFreqIncrement(const casa::CoordinateSystem& c1,
        const casa::CoordinateSystem& c2)
{
    return getChanFreq(c2) - getChanFreq(c1);
}

casa::CoordinateSystem makeCoordinates(const casa::CoordinateSystem& c1,
        const casa::CoordinateSystem& c2,
        const casa::IPosition& refShape)
{
    assertValidCoordinates(c1);
    assertValidCoordinates(c2);
    const int whichSpectral = c1.findCoordinate(casa::Coordinate::SPECTRAL);

    const casa::Vector<casa::Int> axesSpectral = c1.pixelAxes(whichSpectral);
    ASKAPASSERT(casa::uInt(axesSpectral[0]) < refShape.nelements());

    // Copy and update the spectral coordinate
    casa::SpectralCoordinate freq(c1.spectralCoordinate(whichSpectral));
    freq.setReferencePixel(casa::Vector<double>(1, 0.0));
    freq.setReferenceValue(casa::Vector<double>(1, getChanFreq(c1)));
    freq.setIncrement(casa::Vector<double>(1, getFreqIncrement(c1, c2)));

    // Build the coordinate system
    casa::CoordinateSystem csys;
    for (casa::uInt axis = 0; axis < c1.nCoordinates(); ++axis) {
        if (c1.type(axis) != casa::Coordinate::SPECTRAL) {
            csys.addCoordinate(c1.coordinate(axis));
        } else {
            csys.addCoordinate(freq);
        }
    }
    return csys;
}

int main(int argc, char *argv[])
{
    if (argc != 5) {
        std::cerr << "usage: " << argv[0]
            << " <image base name> <range begin> <range end> <output cube name>"
            << std::endl;
        return 1;
    }

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
    casa::LogSinkInterface* globalSink = new askap::Log4cxxLogSink();
    casa::LogSink::globalSink(globalSink);

    StatReporter stats;

    // Parameters
    const std::string imageBase(argv[1]);
    const int rangeBegin = atoi(argv[2]);
    const int rangeEnd = atoi(argv[3]);
    const int nChan = rangeEnd - rangeBegin + 1;
    const std::string name(argv[4]);

    // Name of first image. This image is used to get the coordinate system
    // and the dimensions of the input images. We assume that all these images
    // have the same coordinate systems and dimensions, although that may not
    // be correct
    const casa::PagedImage<float> refImage(getName(imageBase, rangeBegin));
    const casa::IPosition refShape = refImage.shape();
    if (refShape(0) != refShape(1)) {
        ASKAPLOG_INFO_STR(logger, "Error: Input images must be square in i & j dimensions");
        return 1;
    }
    const int xyDims = refShape(0);
    const int nStokes = refShape(2);
    const casa::CoordinateSystem refCoordinates = refImage.coordinates();

    const casa::PagedImage<float> secondImage(getName(imageBase, rangeBegin + 1));
    const casa::CoordinateSystem secondCoordinates = secondImage.coordinates();


    const casa::Unit refUnits = refImage.units();
    const casa::ImageInfo refImageInfo = refImage.imageInfo();
    const casa::CoordinateSystem newCsys = makeCoordinates(refImage.coordinates(),
            secondImage.coordinates(), refShape);

    // Create new image cube
    const casa::IPosition cubeShape(4, xyDims, xyDims, nStokes, nChan);

    const double size = static_cast<double>(xyDims) * xyDims * nStokes * nChan * sizeof(float);
    ASKAPLOG_INFO_STR(logger, "Creating image cube of size ~" << std::setprecision(2)
            << (size / 1024.0 / 1024.0 / 1024.0) << "GB. This may take a few minutes.");

    casa::PagedImage<float> cube(casa::TiledShape(cubeShape), newCsys, name);
    cube.set(0.0);
    cube.setUnits(refUnits);
    cube.setImageInfo(refImageInfo);

    // Open source images and write the slices into the cube
    for (int i = 0; i <= (rangeEnd - rangeBegin); ++i) {
        std::string name = getName(imageBase, i + rangeBegin);
        ASKAPLOG_INFO_STR(logger, "Adding slice from image " << name);
        casa::PagedImage<float> img(name);

        // Ensure shape is the same
        if (img.shape() != refShape) {
            ASKAPLOG_ERROR_STR(logger, "Error: Input images must all have the same shape");
            return 1;
        }

        // Ensure coordinate system is the same
        /*
           if (img.coordinates() != refCoordinates) {
           ASKAPLOG_ERROR_STR(logger, "Error: Input images must all have the same coordinate system");
           return 1;
           }
           */

        // Ensure units are the same
        if (img.units() != refUnits) {
            ASKAPLOG_ERROR_STR(logger, "Error: Input images must all have the same units");
            return 1;
        }

        casa::Array<float> arr = img.get();
        casa::IPosition where(4, 0, 0, 0, i);
        cube.putSlice(arr, where);
    }

    stats.logSummary();
    return 0;
}
