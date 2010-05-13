/// @file
///
/// Provides methods to access data in casa images and store the information in duchamp classes.
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
#ifndef ASKAP_ANALYSIS_CASAIMAGE_H_
#define ASKAP_ANALYSIS_CASAIMAGE_H_

#include <analysisutilities/SubimageDef.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <string>

#include <wcslib/wcs.h>

#include <duchamp/fitsHeader.hh>
#include <duchamp/param.hh>
#include <duchamp/Cubes/cubes.hh>

#include <casa/aipstype.h>
#include <casa/Arrays/Slicer.h>
#include <images/Images/ImageInterface.h>
using namespace casa;

namespace askap {

    namespace analysis {

        /// @ingroup analysisutilities
        /// @brief Save a wcsprm struct to a duchamp::FitsHeader
        void storeWCStoHeader(duchamp::FitsHeader &head, duchamp::Param &par, wcsprm *wcs);

        /// @ingroup analysisutilities
        /// @brief Save a casa image to a duchamp::Cube object
        /// @name
        /// @{
        int casaImageToCube(duchamp::Cube &cube, SubimageDef &subDef, int subimageNumber);
        int casaImageToCubeData(const ImageInterface<Float> *imagePtr, duchamp::Cube &cube);
        /// @}

        /// @brief Return the dimensions of the image
        std::vector<long> getCASAdimensions(std::string filename);

        /// @brief Save the metadata from a casa image to a duchamp::Cube object
        /// @name
        /// @ingroup analysisutilities
        /// @{
        int casaImageToMetadata(duchamp::Cube &cube, SubimageDef &subDef, int subimagenumber);
        int casaImageToMetadata(const ImageInterface<Float> *imagePtr, duchamp::Cube &cube);
        /// @}

        /// @ingroup analysisutilities
        /// @brief Read the beam information from a casa image
        void readBeamInfo(const ImageInterface<Float>* imagePtr, duchamp::FitsHeader &head, duchamp::Param &par);

        /// @ingroup analysisutilities
        /// @brief Extract the WCS information from a casa image
        /// @name
        /// @{
        wcsprm *casaImageToWCS(std::string imageName);
        wcsprm *casaImageToWCS(const ImageInterface<Float>* imagePtr);
        /// @}

        /// @ingroup analysisutilities
	/// @brief Convert a WCS struct to a casa coordinate specification
	casa::CoordinateSystem wcsToCASAcoord(wcsprm *wcs, int nstokes);

        /// @ingroup analysisutilities
        /// @brief Convert a duchamp subsection to a casa Slicer
        Slicer subsectionToSlicer(duchamp::Section &subsection);

	/// @ingroup analysisutilities
	/// @brief Fix axes that aren't position or spectral
	void fixSlicer(Slicer &slice, wcsprm *wcs);

        /// @brief Find the noise within a box surrounding a location in an image.
        float findSurroundingNoise(std::string filename, float xpt, float ypt, int noiseBoxSize);

	/// @brief Return a vector of pixel values in a box subsection of an image.
	casa::Vector<casa::Double> getPixelsInBox(std::string imageName, casa::Slicer box);

    }

}

#endif

