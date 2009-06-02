/// @file
/// @brief Converter of polarisation frames
/// @details This is the class which handles polarisation frame conversion and contains some
/// helper methods related to it (i.e. converting strings into Stokes enums). It may eventually
/// replace or become derived from IPolSelector, which is not used at the moment.
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

#ifndef POL_CONVERTER_H
#define POL_CONVERTER_H

// casa includes
#include <casa/Arrays/Vector.h>
#include <measures/Measures/Stokes.h>

namespace askap {

namespace synthesis {

/// @brief Converter of polarisation frames
/// @details This is the class which handles polarisation frame conversion and contains some
/// helper methods related to it (i.e. converting strings into Stokes enums). It may eventually
/// replace or become derived from IPolSelector, which is not used at the moment.
/// @note At this stage this class is incompatible with the converters used to create data iterator.
/// It is not clear at the moment whether this class should be modified to be used as such converter 
/// too.
/// @ingroup dataaccess_conv
struct PolConverter {

  /// @brief constructor of the converter to a given frame
  /// @details
  /// @param[in] polFrame output polarisation frame defined as a vector of Stokes enums
  PolConverter(const casa::Vector<casa::Stokes::StokesTypes> &polFrame);
  
  /// @brief default constructor - no conversion
  /// @details Constructed via this method the object passes all visibilities intact
  PolConverter();
  
  /// @brief main method doing conversion
  /// @details Convert the given visibility vector from the frame described in polFrame to 
  /// the target polarisation frame
  /// @param[in] polFrame input polarisation frame given as a vector of Stokes enums
  /// @param[in] vis visibility vector
  /// @return converted visibility vector 
  /// @note polFrame and vis should have the same size (<=4), the output vector will have the
  /// same size too.
  casa::Vector<casa::Complex> convert(const casa::Vector<casa::Stokes::StokesTypes> &polFrame,
                     casa::Vector<casa::Complex> vis) const;
private:
  /// @brief no operation flag
  /// @details True, if created with a default constructor, false otherwise
  bool itsVoid;
  
  /// @brief target polarisation frame (stokes enums)
  casa::Vector<casa::Stokes::StokesTypes> itsPolFrame;                     
};

} // namespace synthesis

} // namespace askap

#endif // #ifndef POL_CONVERTER_H
