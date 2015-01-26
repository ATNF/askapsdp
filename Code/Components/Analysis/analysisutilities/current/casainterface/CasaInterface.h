/// @file
///
/// Functions that provide interfaces between CASA images &
/// coordinates and more familiar Duchamp structures. Also provides
/// utility functions that enable rapid access to certain parts of
/// images or coordinate systems.
///
/// @copyright (c) 2007 CSIRO
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#ifndef ASKAP_ANALYSISUTILS_CASA_H_
#define ASKAP_ANALYSISUTILS_CASA_H_

#include <analysisparallel/SubimageDef.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <askapparallel/AskapParallel.h>

#include <string>

#include <wcslib/wcs.h>

#include <duchamp/fitsHeader.hh>
#include <duchamp/param.hh>
#include <duchamp/Cubes/cubes.hh>

#include <casa/aipstype.h>
#include <casa/Arrays/Slicer.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/SubImage.h>
using namespace casa;

namespace askap {

namespace analysisutilities {

/// Return the dimensions of a given image.
std::vector<size_t> getDim(const boost::shared_ptr<ImageInterface<Float> > imagePtr);

/// Check whether an image is able to be opened.
bool imageExists(std::string imagename);

/// Open an image and return an ImageInterface object
const boost::shared_ptr<ImageInterface<Float> > openImage(std::string imagename);

/// Return a subsection of an image, given by a casa::Slicer
const boost::shared_ptr<SubImage<Float> > getSubImage(std::string imagename, Slicer slicer);

/// @ingroup analysisutilities
/// @brief Save a wcsprm struct to a duchamp::FitsHeader
void storeWCStoHeader(duchamp::FitsHeader &head, duchamp::Param &par, wcsprm *wcs);

/// @brief Return a whole-image subsection string for an image
std::string getFullSection(std::string filename);
/// @brief Return the dimensions of the image
std::vector<size_t> getCASAdimensions(std::string filename);

/// @ingroup analysisutilities
/// @brief Read the beam information from a casa image
void readBeamInfo(const boost::shared_ptr<ImageInterface<Float> > imagePtr,
                  duchamp::FitsHeader &head, duchamp::Param &par);

/// @ingroup analysisutilities
/// @brief Extract the WCS information from a casa image
/// @name
/// @{
wcsprm *casaImageToWCS(std::string imageName);
wcsprm *casaImageToWCS(const boost::shared_ptr<ImageInterface<Float> > imagePtr);
/// @}

/// @ingroup analysisutilities
/// @brief Convert a WCS struct to a casa coordinate specification
casa::CoordinateSystem wcsToCASAcoord(wcsprm *wcs, int nstokes);

/// @ingroup analysisutilities
/// @brief Convert a duchamp subsection to a casa Slicer
Slicer subsectionToSlicer(duchamp::Section &subsection);
Slicer subsectionToSlicer(duchamp::Section &subsection, wcsprm *wcs);

/// @ingroup analysisutilities
/// @brief Fix axes that aren't position or spectral
void fixSlicer(Slicer &slice, wcsprm *wcs);

/// @brief Find the noise within a box surrounding a location in an image.
float findSurroundingNoise(std::string filename, float xpt, float ypt, int noiseBoxSize);

/// @brief Return a vector of pixel values in a box subsection of an image.
casa::Array<casa::Float> getPixelsInBox(std::string imageName, casa::Slicer box,
                                        bool fixSlicer = true);

/// @brief Increase the length of a Slicer by adding degenerate dimensions on the end
void lengthenSlicer(Slicer &slice, int ndim);
}

}

#endif

