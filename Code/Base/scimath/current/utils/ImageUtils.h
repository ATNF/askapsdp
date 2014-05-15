/// @file
/// @brief Helper method(s) to work with casa images
/// @details This file contains methods which are largely used for debugging. This is the reason
/// why we want to have them at the high enough level. It is envisaged that methods will be moved
/// here from SynthesisParamsHelper as required.
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef ASKAP_SCIMATH_IMAGE_UTILS_H
#define ASKAP_SCIMATH_IMAGE_UTILS_H

// casa includes
#include <casa/aips.h>
#include <casa/Arrays/Array.h>


namespace askap {

namespace scimath {

/// @brief save a 2D array as a CASA image
/// @details This method is intended to be used largely for debugging. To save image from
/// parameter class use loadImageParameter method
/// @param[in] imagename name of the output image file
/// @param[in] arr input array
void saveAsCasaImage(const std::string &imagename, const casa::Array<casa::Float> &arr);


} // namespace scimath

} // namespace askap

#endif // #ifndef ASKAP_SCIMATH_IMAGE_UTILS_H

