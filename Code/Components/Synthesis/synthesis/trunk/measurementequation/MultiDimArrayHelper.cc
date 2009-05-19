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

#include <measurementequation/MultiDimArrayHelper.h>
#include <askap/AskapError.h>
#include <askap/AskapUtil.h>

using namespace askap;
using namespace synthesis;

/// @brief setup the iterator
/// @details 
/// @param[in] shape shape of the full hypercube (or array-valued parameter) 
MultiDimArrayHelper::MultiDimArrayHelper(const casa::IPosition &shape) : 
     ArrayPositionIterator(shape,casa::IPosition(shape.nelements(),0),2u), itsShape(shape),
     itsPlaneShape(shape), itsSequenceNumber(0)
{
  ASKAPASSERT(itsPlaneShape.nelements()>=2);
  for (casa::uInt dim=2; dim<itsPlaneShape.nelements(); ++dim) {
       itsPlaneShape[dim] = 1;
  }
}
   
/// @brief extract a single plane from an array
/// @details This method forms a slice of the given array to extract a single plane corresponding
/// to the current position of the iterator
/// @param[in] in input array
/// @return output array (single plane)
casa::Array<double> MultiDimArrayHelper::getPlane(casa::Array<double> &in) const
{
  return in;
}

/// @brief extract a single plane form a 1D array
/// @This method extracts a single slice from an array flattened to a 1D vector. The slice 
/// corresponds to the current position of the iterator. This method preserves the degenerate
/// dimensions.
/// @param[in] in input vector
/// @return output array (single plane)
casa::Array<double> MultiDimArrayHelper::getPlane(casa::Vector<double> &in) const
{
  ASKAPDEBUGASSERT(itsShape.conform(in.shape())); 
  casa::Array<double> reformedReference = in.reform(itsShape);
  return getPlane(reformedReference);
}

/// @brief return the unique tag of the current plane
/// @details To assist caching one may need a string key which is unique for every iteration.
/// This method forms a string tag from the position vector, which can be appended to the
/// parameter name to get a unique string for every single plane.
/// @note This is an alternative way to converting sequenceNumber to string.
/// @return string tag
std::string MultiDimArrayHelper::tag() const
{
  return utility::toString<casa::uInt>(sequenceNumber());
}

