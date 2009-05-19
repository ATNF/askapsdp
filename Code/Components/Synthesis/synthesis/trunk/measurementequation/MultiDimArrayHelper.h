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

#include <string>

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
   
   /// @brief extract a single plane form a 1D array
   /// @This method extracts a single slice from an array flattened to a 1D vector. The slice 
   /// corresponds to the current position of the iterator. This method preserves the degenerate
   /// dimensions.
   /// @param[in] in input vector
   /// @return output array (single plane)
   casa::Array<double> getPlane(casa::Vector<double> &in) const;
   
   /// @brief return the sequence number of the plane
   /// @details To assist with caching this method returns consequitive numbers for every
   /// iteration. The first iteration corresponds to 0.
   /// @return sequence number
   inline casa::uInt sequenceNumber() const { return itsSequenceNumber;}
   
   /// @brief return the unique tag of the current plane
   /// @details To assist caching one may need a string key which is unique for every iteration.
   /// This method forms a string tag from the position vector, which can be appended to the
   /// parameter name to get a unique string for every single plane.
   /// @note This is an alternative way to converting sequenceNumber to string.
   /// @return string tag
   std::string tag() const;
   
   /// @brief obtain a shape of the single plane
   /// @details This method returns the shape of a sinlge plane preserving degenerate
   /// dimensions. 
   /// @return a shape of the single plane
   inline const casa::IPosition& planeShape() const { return itsPlaneShape;}
   
   /// @brief shape of the full array
   /// @return shape of the full array
   inline const casa::IPosition& shape() const { return itsShape;}
   
   /// @brief obtain current position within the whole array
   /// @details This method returns the bottom left corner (blc) of the current plane
   /// @return blc of the current plane
   inline const casa::IPosition& position() const { return pos();}
   
   /// @brief check whether there are more planes to iterate
   /// @details
   /// @return true, if the iteration is not complete
   inline bool hasMore() const { return !pastEnd();}
   
   /// @brief proceed to the next iteration
   /// @details A call to this method makes a step of the iterator
   virtual void next();
   
private:
   /// @brief shape of the full hypercube
   casa::IPosition itsShape;    

   /// @brief shape of a single plane of the hypercube
   /// @details To relieve the user of this class from repeated similar operations this variable
   /// stores the shape of all degenerate dimensions preserved (i.e. [x,y,1,1])
   casa::IPosition itsPlaneShape;
   
   /// @brief sequence number
   casa::uInt itsSequenceNumber;
};

} // namespace synthesis

} // namespace askap


#endif // #ifndef MULTI_DIM_ARRAY_HELPER_H

