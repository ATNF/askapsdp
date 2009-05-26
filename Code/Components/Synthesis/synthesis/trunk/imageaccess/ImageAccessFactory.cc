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

#include <imageaccess/ImageAccessFactory.h>
#include <imageaccess/CasaImageAccess.h>

#include <askap/AskapError.h>

#include <string>

using namespace askap;
using namespace askap::synthesis;

/// @brief Build an appropriate image access class
/// @details This is a factory method generating a shared pointer to the image
/// accessor from the parset file
/// @param[in] parset parameters containing description of image accessor to be constructed
/// @return shared pointer to the image access object
/// @note CASA images are used by default 
boost::shared_ptr<IImageAccess> askap::synthesis::imageAccessFactory(const LOFAR::ACC::APS::ParameterSet &parset)
{
   std::string imageType = parset.getString("imagetype","casa");
   boost::shared_ptr<IImageAccess> result;
   if (imageType == "casa") {
       boost::shared_ptr<CasaImageAccess> iaCASA(new CasaImageAccess());
       // optional parameter setting may come here
       result = iaCASA;
   } else {
      throw AskapError(std::string("Unsupported image type ")+imageType+" has been requested"); 
   }
   return result;
}
