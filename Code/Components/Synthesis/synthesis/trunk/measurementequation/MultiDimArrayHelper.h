/// @file
/// 
/// @brief helper class to assist with spectral line and polarisation images
/// @details Images are represented as array-valued parameters. Constituents of
/// the normal equations are just single-dimension vectors. The images may actually
/// be hypercubes (polarisation and spectral dimensions). This class facilitates
/// iterations over such images (plane by plane).
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

#ifndef MULTI_DIM_ARRAY_HELPER_H
#define MULTI_DIM_ARRAY_HELPER_H

#include <casa/Arrays/ArrayPosIter.h>
#include <casa/Arrays/Array.h>

namespace askap {

namespace synthesis {

/// @brief helper class to assist with spectral line and polarisation images
/// @details Images are represented as array-valued parameters. Constituents of
/// the normal equations are just single-dimension vectors. The images may actually
/// be hypercubes (polarisation and spectral dimensions). This class facilitates
/// iterations over such images (plane by plane). 
/// @note This class is relatively generic and can be moved to a higher level (i.e. to Base), 
/// if needed somewhere else.
/// @ingroup measurementequation
struct MultiDimArrayHelper : protected casa::ArrayPositionIterator {
   
   /// @brief setup the iterator
   /// @details 
   /// @param[in] shape shape of the full hypercube (or array-valued parameter) 
   MultiDimArrayHelper(const casa::IPosition &shape);
   
   /// @brief extract a single plane from an array
   /// @details This method forms a slice of the given array to extract a single plane corresponding
   /// to the current position of the iterator
   /// @param[in] in input array
   /// @return output array (single plane)
   casa::Array<double> getPlane(casa::Array<double> &in) const;
   
   
private:
   /// @brief shape of the full hypercube
   casa::IPosition itsShape;    

   /// @brief shape of a single plane of the hypercube
   /// @details To relieve the user of this class from repeated similar operations this variable
   /// stores the shape of all degenerate dimensions preserved (i.e. [x,y,1,1])
   casa::IPosition itsPlaneShape;
};

} // namespace synthesis

} // namespace askap


#endif // #ifndef MULTI_DIM_ARRAY_HELPER_H

