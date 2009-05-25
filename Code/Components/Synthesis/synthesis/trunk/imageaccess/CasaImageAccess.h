/// @file
/// @brief Access casa image
/// @details This class implements IImageAccess interface for CASA image
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

#ifndef CASA_IMAGE_ACCESS_H
#define CASA_IMAGE_ACCESS_H

#include <imageaccess/IImageAccess.h>

namespace askap {

namespace synthesis {

/// @brief Access casa image
/// @details This class implements IImageAccess interface for CASA image
/// @ingroup imageaccess
struct CasaImageAccess {

  // reading methods
  
  /// @brief obtain the shape
  /// @param[in] name image name
  /// @return full shape of the given image
  virtual casa::IPosition shape(const std::string &name) const;
  
  /// @brief read full image
  /// @param[in] name image name
  /// @return array with pixels
  virtual casa::Array<float> read(const std::string &name) const;
  
  /// @brief read part of the image 
  /// @param[in] name image name
  /// @param[in] blc bottom left corner of the selection
  /// @param[in] trc top right corner of the selection
  /// @return array with pixels for the selection only
  virtual casa::Array<float> read(const std::string &name, const casa::IPosition &blc,
                                  const casa::IPosition &trc) const;
  
  /// @brief obtain coordinate system info
  /// @param[in] name image name
  /// @return coordinate system object
  virtual casa::CoordinateSystem coordSys(const std::string &name) const;
  
  // writing methods
  
  /// @brief create a new image
  /// @details A call to this method should preceed any write calls. The actual
  /// image may be created only upon the first write call. Details depend on the
  /// implementation. 
  /// @param[in] name image name
  /// @param[in] shape full shape of the image
  /// @param[in] csys coordinate system of the full image
  virtual void create(const std::string &name, const casa::IPosition &shape, 
                      const casa::CoordinateSystem &csys);
                      
  /// @brief write full image
  /// @param[in] name image name
  /// @param[in] arr array with pixels
  virtual void write(const std::string &name, const casa::Array<float> &arr);
  
  /// @brief write a slice of an image
  /// @param[in] name image name
  /// @param[in] arr array with pixels
  /// @param[in] where bottom left corner where to put the slice to (trc is deduced from the array shape)
  virtual void write(const std::string &name, const casa::Array<float> &arr, 
               const casa::IPosition &where);                    
}; 


} // namespace synthesis

} // namespace askap

#endif // #ifndef CASA_IMAGE_ACCESS_H

