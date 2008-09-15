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
#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <string>

#include <wcslib/wcs.h>

#include <duchamp/fitsHeader.hh>
#include <duchamp/param.hh>
#include <duchamp/Cubes/cubes.hh>

#include <casa/aipstype.h>
#include <images/Images/ImageInterface.h>
using namespace casa;

namespace askap
{

  namespace analysis
  {

    /// @ingroup analysisutilities
    /// @brief Save a wcsprm struct to a duchamp::FitsHeader
    void storeWCStoHeader(duchamp::FitsHeader &head, duchamp::Param &par, wcsprm *wcs);

    /// @ingroup analysisutilities
    /// @brief Save a casa image to a duchamp::Cube object
    /// @name
    /// @{
    int casaImageToCube(duchamp::Cube &cube);
    int casaImageToCubeData(ImageInterface<Float> *imagePtr, duchamp::Cube &cube);
    /// @}

    /// @brief Save the metadata from a casa image to a duchamp::Cube object
    /// @name 
    /// @ingroup analysisutilities
    /// @{
    int casaImageToMetadata(duchamp::Cube &cube);
    int casaImageToMetadata(ImageInterface<Float> *imagePtr, duchamp::Cube &cube);
    /// @}

    /// @ingroup analysisutilities
    /// @brief Read the beam information from a casa image
    void readBeamInfo(ImageInterface<Float>* imagePtr, duchamp::FitsHeader &head, duchamp::Param &par);

    /// @ingroup analysisutilities
    /// @brief Extract the WCS information from a casa image
    /// @name
    /// @{
    wcsprm *casaImageToWCS(std::string imageName);
    wcsprm *casaImageToWCS(ImageInterface<Float>* imagePtr);
    /// @}

  }

}
