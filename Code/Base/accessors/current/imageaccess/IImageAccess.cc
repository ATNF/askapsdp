/// @file
/// @brief Basic interface to access an image
/// @details This interface class is somewhat analogous to casa::ImageInterface. But it has
/// only methods we need for synthesis and allow more functionality to access a part of the image.
/// In the future we can benefit from using this minimalistic interface because it should be 
/// relatively easy to do parallel operations on the same image or even distributed storage.
///  
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

#include <imageaccess/IImageAccess.h>

namespace askap {

namespace synthesis {

/// @brief void virtual desctructor, to keep the compiler happy
IImageAccess::~IImageAccess() {}

} // namespace synthesis

} // namespace askap

