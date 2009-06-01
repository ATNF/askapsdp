/// @file
/// @brief An interface to POLARIZATION subtable
/// @details A class derived from this interface provides access to
/// the content of POLARIZATION subtable (which describes which products were
/// measured). The table is indexed with the polarisation ID, which can be obtained from
/// the data descriptor ID and the appropriate table.
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

#ifndef I_TABLE_POLARISATION_HOLDER_H
#define I_TABLE_POLARISATION_HOLDER_H

// casa includes
#include <casa/Arrays/Vector.h>
#include <measures/Measures/Stokes.h>

// own includes
#include <dataaccess/IHolder.h>

namespace askap {

namespace synthesis {

/// @brief An interface to POLARIZATION subtable
/// @details A class derived from this interface provides access to
/// the content of POLARIZATION subtable (which describes which products were
/// measured). The table is indexed with the polarisation ID, which can be obtained from
/// the data descriptor ID and the appropriate table.
/// @ingroup dataaccess_tab
struct ITableSpWindowHolder : virtual public IHolder {
   
   /// @brief number of polarisation products for the given ID
   /// @param[in] polID polarisation ID of interest
   /// @return number of products for the given ID
   virtual size_t nPol(casa::uInt polID) const = 0;
   
   /// @brief obtain polarisation types for the given ID
   /// @param[in] polID polarisation ID of interest
   /// @return a vector (size is nPol) with types of polarisation products, same order as in the
   /// visibility cube
   virtual casa::Vector<casa::Stokes> getTypes(casa::uInt polID) const = 0;
   
   /// @brief obtain polarisation type of a single polarisation product
   /// @details This version of the method extracts type for just one polarisation product.
   /// @param[in] polID polarisation ID of interest
   /// @param[in] pol polarisation product (should be less than nPol)
   /// @return a type of the polarisation product given as casa::Stokes
   virtual casa::Stokes getType(casa::uInt polID, casa::uInt pol) const = 0;
};

} // namespace synthesis

} // namespace askap

#endif // #ifndef I_TABLE_POLARISATION_HOLDER_H


