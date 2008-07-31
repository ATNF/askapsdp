/// @file
/// 
/// @brief utilities related to illumination pattern
/// @details This class is written for experiments with eigenbeams and synthetic beams.
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

#ifndef ILLUMINATION_UTILS_H
#define ILLUMINATION_UTILS_H

#include <boost/shared_ptr.hpp>
#include <gridding/IBasicIllumination.h>

#include <string>

namespace askap {

namespace synthutils {

/// @brief utilities related to illumination pattern
/// @details This class is written for experiments with eigenbeams and synthetic beams.
class IlluminationUtils {
public:
   /// @brief constructor
   /// @details 
   /// @param[in] illum illumination pattern to work with   
   /// @param[in] size desired image size
   /// @param[in] cellsize uv-cell size
   /// @param[in] oversample oversampling factor (default 1)
   IlluminationUtils(const boost::shared_ptr<synthesis::IBasicIllumination> &illum,
                     size_t size, double cellsize, size_t oversample);
   
   
   /// @brief save the pattern into an image
   /// @details name file name
   void save(const std::string &name);
   
private:
   /// @brief illumination pattern corresponding to the single feed
   boost::shared_ptr<synthesis::IBasicIllumination> itsIllumination;
   
   /// @brief size of the pattern to work with 
   size_t itsSize;
   
   /// @brief required cell size of the pixellized pattern (wavelengths)
   double itsCellSize;
   
   /// @brief oversampling factor
   size_t itsOverSample;
};

} // namespace synthutils

} // namespace askap

#endif // #ifndef ILLUMINATION_UTILS_H
