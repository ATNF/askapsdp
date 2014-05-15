/// @file CubeMaker.cc
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>

// Include own header file first
#include <makecube/CubeMaker.h>

// Include package level header file
#include <askap_pipelinetasks.h>

// System includes
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

// ASKAPsoft includes
#include <askap/AskapError.h>
#include <askap/AskapLogging.h>
#include <imageaccess/BeamLogger.h>
#include <boost/scoped_ptr.hpp>
#include <Common/ParameterSet.h>
#include <casa/Arrays/IPosition.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <images/Images/PagedImage.h>
#include <casa/Quanta/Unit.h>

// Local package includes
#include <makecube/CubeMakerHelperFunctions.h>

ASKAP_LOGGER(logger, ".CubeMaker");

using namespace askap::cp::pipelinetasks;

/// @details
/// Read the input parameters from the ParameterSet. Accepted parameters:
/// 'inputNamePattern', 'outputCube', 'restFrequency', 'beamReference', 'beamLog'.
/// Also initialises the cube pointer to zero.
CubeMaker::CubeMaker(const LOFAR::ParameterSet& parset)
    : itsInputNamePattern(parset.getString("inputNamePattern", "")),
      itsCubeName(parset.getString("outputCube", "")),
      itsBeamReference(parset.getString("beamReference", "mid")),
      itsBeamLog(parset.getString("beamLog", ""))
{
    const std::string restFreqString = parset.getString("restFrequency", "-1.");
    if (restFreqString == "HI") {
        itsRestFrequency = REST_FREQ_HI;
    } else {
        itsRestFrequency = atof(restFreqString.c_str());
    }
}

CubeMaker::~CubeMaker()
{
}

/// @details
/// Takes the input name pattern and expands to a vector list of input filenames,
/// using the expandPattern function. Parses the beamReference parameter to get
/// the image number from which to read the beam information that will be stored
/// in the output cube. Calls getReferenceData().
void CubeMaker::initialise()
{
    itsInputNames = CubeMakerHelperFunctions::expandPattern(itsInputNamePattern);

    if (itsInputNames.size() < 2) ASKAPTHROW(AskapError, "Insufficient input files");

    itsNumChan = itsInputNames.size();

    if (itsBeamReference == "mid") {
        itsBeamImageNum = itsNumChan / 2;
    } else if (itsBeamReference == "first") {
        itsBeamImageNum = 0;
    } else if (itsBeamReference == "last") {
        itsBeamImageNum = itsNumChan - 1;
    } else {
        int num = atoi(itsBeamReference.c_str());

        if (num >= 0 && num < itsNumChan) {
            itsBeamImageNum = size_t(num);
        } else {
            ASKAPLOG_WARN_STR(logger, "beamReference value (" << itsBeamReference << ") not valid. Using middle value of " << itsNumChan / 2);
            itsBeamImageNum = itsNumChan / 2;
        }
    }

    getReferenceData();
}

/// @details
/// The reference data details the shape of the input images, their units and
/// coordinates. These are used for construction of the cube and verification
/// of all input images. The reference data is read from the first image in the
/// vector list. The coordinate system of the second image in that list is also
/// extracted - the spectral increment will be determined from these two coordinate
/// systems.
void CubeMaker::getReferenceData()
{
    const casa::PagedImage<float> refImage(itsInputNames[0]);
    itsRefShape = refImage.shape();
    itsRefCoordinates = refImage.coordinates();
    itsRefUnits = refImage.units();

    const casa::PagedImage<float> secondImage(itsInputNames[1]);
    itsSecondCoordinates = secondImage.coordinates();
}

/// @details
/// The coordinate system for the cube is constructed using the makeCoordinates
/// function. If required, the rest frequency is added. The cube is then created
/// using the reference shape and the number of channels in the input file list.
void CubeMaker::createCube()
{
    casa::CoordinateSystem newCsys = CubeMakerHelperFunctions::makeCoordinates(
            itsRefCoordinates, itsSecondCoordinates, itsRefShape);

    if (itsRestFrequency > 0.) setRestFreq(newCsys);

    const casa::IPosition cubeShape(4, itsRefShape(0), itsRefShape(1), itsRefShape(2), itsNumChan);
    const double size = static_cast<double>(cubeShape.product()) * sizeof(float);
    ASKAPLOG_INFO_STR(logger, "Creating image cube " << itsCubeName
                      << "  of size approximately " << std::setprecision(2)
                      << (size / 1024.0 / 1024.0 / 1024.0) << "GB. This may take a few minutes.");

    itsCube.reset(new casa::PagedImage<float>(casa::TiledShape(cubeShape), newCsys, itsCubeName));
}

/// @details
/// The rest frequency, as provided in the input parameter set, is added to the
/// coordinate system, replacing any previous value that is already there.
///
/// @param csys  The coordinate system to which the
///              rest frequency is to be added.
void CubeMaker::setRestFreq(casa::CoordinateSystem& csys)
{

    CubeMakerHelperFunctions::assertValidCoordinates(csys);
    const int whichSpectral = csys.findCoordinate(casa::Coordinate::SPECTRAL);
    casa::SpectralCoordinate speccoord = csys.spectralCoordinate(whichSpectral);

    if (!speccoord.setRestFrequency(itsRestFrequency)) {
        ASKAPLOG_ERROR_STR(logger, "Could not set the rest frequency to " << itsRestFrequency);
    } else {
        if (!csys.replaceCoordinate(speccoord, whichSpectral)) {
            ASKAPLOG_ERROR_STR(logger,
                    "Could not set the rest frequency - error replacing the spectral coordinates");
        }
    }
}

/// @details
/// If the output cube has been created, the reference units and the requested
/// reference beam shape are added to the cube.
void CubeMaker::setImageInfo()
{
    if (itsCube.get()) {
        itsCube->setUnits(itsRefUnits);
        casa::PagedImage<float> midImage(itsInputNames[itsBeamImageNum]);
        itsCube->setImageInfo(midImage.imageInfo());
    }
}

/// @details
/// Each input channel image is added in order to the output cube.
void CubeMaker::writeSlices()
{
    for (size_t i = 0; i < itsInputNames.size(); ++i) {
        if (!writeSlice(i))
            ASKAPTHROW(AskapError, "Could not write slice #" << i);
    }
}

//// @details
/// An individual channel image is added to the cube in the appropriate location.
/// Checks are performed to verify that the channel image has the same shape and
/// units as the reference (ie. the first in the vector list), and has compatible
/// coordinates (as defined by the compatibleCoordinates function).
/// 
/// @param[in] i The number of the image in the vector list
///              of input images.
///
/// @return  Returns true if things work. If any checks fail, the index is out
///          of bounds, or the cube is not yet open, then false is returned
///          (and an ERROR log message written).
bool CubeMaker::writeSlice(size_t i)
{
    if (itsCube.get()) {
        if (i > itsInputNames.size()) {
            ASKAPLOG_ERROR_STR(logger, "writeSlice - index " << i << " out of bounds");
            return false;
        }

        ASKAPLOG_INFO_STR(logger, "Adding slice from image " << itsInputNames[i]);
        casa::PagedImage<float> img(itsInputNames[i]);

        // Ensure shape is the same
        if (img.shape() != itsRefShape) {
            ASKAPLOG_ERROR_STR(logger, "Error: Input images must all have the same shape");
            return false;
        }

        // Ensure coordinate system is the same
        if (!CubeMakerHelperFunctions::compatibleCoordinates(img.coordinates(),
                    itsRefCoordinates)) {
            ASKAPLOG_ERROR_STR(logger,
                               "Error: Input images must all have compatible coordinate systems");
            return false;
        }

        // Ensure units are the same
        if (img.units() != itsRefUnits) {
            ASKAPLOG_ERROR_STR(logger, "Error: Input images must all have the same units");
            return false;
        }

        casa::Array<float> arr = img.get();
        casa::IPosition where(4, 0, 0, 0, i);
        itsCube->putSlice(arr, where);
        return true;

    } else {
        ASKAPLOG_ERROR_STR(logger, "Cube not open");
        return false;
    }
}

/// @details
/// The beam shape for each input image is written to an ascii file (given by the
/// beamLog input parameter). Each line corresponds to one file, and has columns:
/// number | image name | major axis [arcsec] | minor axis [arcsec] | position
/// angle [deg].
/// Columns are separated by a single space.
void CubeMaker::recordBeams()
{
    if (itsBeamLog != "") {
	const casa::PagedImage<float> firstimg(itsInputNames[0]);
	const casa::Vector<Quantum<Double> > firstbeam = firstimg.imageInfo().restoringBeam();

        if (firstbeam.size() == 0) {
            ASKAPLOG_WARN_STR(logger, "The first input image " << itsInputNames[0]
                    << " has no beam, so not making the beamLog " << itsBeamLog);
        } else {
	    accessors::BeamLogger beamlog(itsBeamLog);
	    beamlog.extractBeams(itsInputNames);
	    beamlog.write();
        }
    }
}
