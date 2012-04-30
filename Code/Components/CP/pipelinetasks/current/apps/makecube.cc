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
#include <libgen.h>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "askap/Log4cxxLogSink.h"
#include "askap/StatReporter.h"
#include "boost/regex.hpp"
#include "casa/Logging/LogIO.h"
#include "casa/Logging/LogSinkInterface.h"
#include "casa/Arrays/Array.h"
#include "casa/Quanta/Unit.h"
#include "coordinates/Coordinates/CoordinateSystem.h"
#include "coordinates/Coordinates/SpectralCoordinate.h"
#include "images/Images/PagedImage.h"

using namespace askap;
using namespace askap::utility;

ASKAP_LOGGER(logger, ".makecube");

// Prints usage information to stderr
void usage(const std::string& basename)
{
    std::cerr << "usage:   " << basename
        << " <input filename pattern> <output cube name>" << std::endl;
    std::cerr << "example: " << basename
        << " \"image.i.[0..7].spectral\" spectral.i.cube" << std::endl;
}

/// Expands the string such as: "image.i.[0..15].spectral" into a vector of
/// strings from: "image.i.0.spectral" to "image.i.15.spectral"
///
/// @param[in] pattern  a string containing the pattern of the image filesnames.
///                     See the description for an example of this pattern.
/// @return A vector of filenames, that is, the expansion of the input pattern.
/// @throw AskapError   If the input pattern is inavlid.
std::vector<std::string> expandPattern(const std::string& pattern)
{
    // Find the prefix (i.e. "image.i.") in the above example
    const size_t openBracketPos = pattern.find_first_of("[");
    if (openBracketPos == string::npos) {
        ASKAPTHROW(AskapError, "Could not find [ in valid range expression");
    }
    const std::string prefix = pattern.substr(0, openBracketPos);

    // Find the suffix (i.e. ".spectral") in the above example
    const size_t closeBracketPos = pattern.find_first_of("]");
    if (closeBracketPos == string::npos) {
        ASKAPTHROW(AskapError, "Could not find ] in valid range expression");
    }
    const std::string suffix = pattern.substr(closeBracketPos + 1, pattern.length());

    // Find the [n..n] pattern
    const boost::regex expr(".*\\[([\\d]+)\\.\\.([\\d]+)\\].*");

    boost::smatch what;
    int begin = -1;
    int end = -1;

    if (regex_match(pattern, what, expr)) {
        begin = fromString<int>(what[1]);
        end = fromString<int>(what[2]);
    } else {
        ASKAPTHROW(AskapError, "Could not find range expression");
    }

    std::vector<std::string> filenames;
    for (int i = begin; i <= end; ++i) {
        std::stringstream ss;
        ss << prefix << i << suffix;
        filenames.push_back(ss.str());
    }

    return filenames;
}

bool compatibleCoordinates(const casa::CoordinateSystem& c1,
                           const casa::CoordinateSystem& c2)
{
    return ((c1.nCoordinates() == c2.nCoordinates())
             && c1.type() == c2.type()
             && c1.nPixelAxes() == c2.nPixelAxes()
             && c1.nWorldAxes() == c2.nWorldAxes()
             && c1.findCoordinate(casa::Coordinate::SPECTRAL) == c2.findCoordinate(casa::Coordinate::SPECTRAL)
             && c1.findCoordinate(casa::Coordinate::STOKES) == c2.findCoordinate(casa::Coordinate::STOKES)
             && c1.findCoordinate(casa::Coordinate::DIRECTION) == c2.findCoordinate(casa::Coordinate::DIRECTION));
}

void assertValidCoordinates(const casa::CoordinateSystem& csys)
{
    const int whichSpectral = csys.findCoordinate(casa::Coordinate::SPECTRAL);
    ASKAPCHECK(whichSpectral > -1,
               "No spectral coordinate present in the coordinate system of the first image.");

    const casa::Vector<casa::Int> axesSpectral = csys.pixelAxes(whichSpectral);
    ASKAPCHECK(axesSpectral.nelements() == 1, "Spectral axis " << whichSpectral
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
    // Ensure correct number of args passed
    if (argc != 3) {
        usage(basename(argv[0]));
        return 1;
    }

    // Convert the input filename pattern into a vector of filenames
    std::vector<std::string> inputnames;
    try {
        inputnames = expandPattern(std::string(argv[1]));
    } catch (AskapError& e) {
        usage(basename(argv[0]));
        return 1;
    }

    if (inputnames.size() < 2) {
        std::cerr << "Error: Insufficient input files" << std::endl;
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

    // Get reference data. This will be used to construct the cube, and later
    // verified to be consistant in the rest of the images
    const casa::PagedImage<float> refImage(inputnames[0]);
    const casa::IPosition refShape = refImage.shape();
    const int nChan = inputnames.size();
    const std::string cubename(argv[2]);
    const casa::CoordinateSystem refCoordinates = refImage.coordinates();
    const casa::Unit refUnits = refImage.units();

    // Coordinates from the second image are needed to work out the
    // frequency increment
    casa::CoordinateSystem secondCoordinates;
    {
        const casa::PagedImage<float> secondImage(inputnames[1]);
        secondCoordinates = secondImage.coordinates();
    }

    // Create new image cube
    const casa::CoordinateSystem newCsys = makeCoordinates(refCoordinates,
                                           secondCoordinates, refShape);
    const casa::IPosition cubeShape(4, refShape(0), refShape(1), refShape(2), nChan);
    const double size = static_cast<double>(cubeShape.product()) * sizeof(float);
    ASKAPLOG_INFO_STR(logger, "Creating image cube of size approximatly " << std::setprecision(2)
                          << (size / 1024.0 / 1024.0 / 1024.0) << "GB. This may take a few minutes.");

    casa::PagedImage<float> cube(casa::TiledShape(cubeShape), newCsys, cubename);
    cube.setUnits(refUnits);

    // Get and set the image info (specifically restoring beam) from the image
    // at the midpoint. The restoring beam is frequency dependent however the
    // ImageInfo only supports a single value. To minimise error, we thus
    // select the midpoint.
    {
        casa::PagedImage<float> midImage(inputnames[inputnames.size() / 2]);
        cube.setImageInfo(midImage.imageInfo());
    }

    // Open source images and write the slices into the cube
    for (size_t i = 0; i < inputnames.size(); ++i) {
        ASKAPLOG_INFO_STR(logger, "Adding slice from image " << inputnames[i]);
        casa::PagedImage<float> img(inputnames[i]);

        // Ensure shape is the same
        if (img.shape() != refShape) {
            ASKAPLOG_ERROR_STR(logger, "Error: Input images must all have the same shape");
            return 1;
        }

        // Ensure coordinate system is the same
        if (!compatibleCoordinates(img.coordinates(), refCoordinates)) {
            ASKAPLOG_ERROR_STR(logger,
                               "Error: Input images must all have compatible same coordinate systems");
            return 1;
        }

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
