/// @file
/// @brief Build an appropriate image access class
/// @details This file contains a factory method generating a shared pointer to the image
/// accessor from the parset file
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

#ifndef IMAGE_ACCESS_FACTORY_H
#define IMAGE_ACCESS_FACTORY_H

#include <Common/ParameterSet.h>
#include <boost/shared_ptr.hpp>

#include <imageaccess/IImageAccess.h>

namespace askap {

namespace synthesis {

/// @brief Build an appropriate image access class
/// @details This is a factory method generating a shared pointer to the image
/// accessor from the parset file
/// @param[in] parset parameters containing description of image accessor to be constructed
/// @return shared pointer to the image access object
/// @note CASA images are used by default 
boost::shared_ptr<IImageAccess> imageAccessFactory(const LOFAR::ParameterSet &parset);

} // namespace synthesis

} // namespace askap

#endif // #ifndef IMAGE_ACCESS_FACTORY_H

