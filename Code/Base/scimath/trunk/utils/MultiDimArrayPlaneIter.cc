/// @file
/// 
/// @brief helper iterator class to assist with spectral line and polarisation images
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

#include <utils/MultiDimArrayPlaneIter.h>
#include <askap/AskapError.h>
#include <askap/AskapUtil.h>

using namespace askap;
using namespace askap::scimath;

/// @brief setup the iterator
/// @details 
/// @param[in] shape shape of the full hypercube (or array-valued parameter) 
MultiDimArrayPlaneIter::MultiDimArrayPlaneIter(const casa::IPosition &shape) : 
     ArrayPositionIterator(shape,casa::IPosition(shape.nelements(),0),2u), itsShape(shape),
     itsPlaneShape(shape), itsSequenceNumber(0)
{
  ASKAPASSERT(itsPlaneShape.nelements()>=2);
  ASKAPASSERT(itsShape.product()>0);
  ASKAPDEBUGASSERT(itsShape[0]>0 && itsShape[1]>0);
  for (casa::uInt dim=2; dim<itsPlaneShape.nelements(); ++dim) {
       ASKAPDEBUGASSERT(itsShape[dim]>0);
       itsPlaneShape[dim] = 1;
  }
}
   
/// @brief extract a single plane from an array
/// @details This method forms a slice of the given array to extract a single plane corresponding
/// to the current position of the iterator
/// @param[in] in input array
/// @return output array (single plane)
casa::Array<double> MultiDimArrayPlaneIter::getPlane(casa::Array<double> &in) const
{
  // we may need to add more functionality to this method to take care of situations
  // when the PSF is defined for a single polarisation/channel only
  const casa::IPosition blc(position());
  casa::IPosition trc(blc);
  trc += itsPlaneShape;
  for (casa::uInt dim = 0; dim<trc.nelements(); ++dim) {
       trc[dim] -= 1;
       ASKAPDEBUGASSERT(trc[dim]<itsShape[dim]);
  }
  return in(blc,trc);
}

/// @brief extract a single plane form a 1D array
/// @This method extracts a single slice from an array flattened to a 1D vector. The slice 
/// corresponds to the current position of the iterator. This method preserves the degenerate
/// dimensions.
/// @param[in] in input vector
/// @return output array (single plane)
casa::Array<double> MultiDimArrayPlaneIter::getPlane(casa::Vector<double> &in) const
{
  ASKAPDEBUGASSERT(itsShape.product() == in.shape().product()); 
  casa::Array<double> reformedReference = in.reform(itsShape);
  return getPlane(reformedReference);
}

/// @brief extract a single plane into a flattened vector
/// @details This method extracts a single plane slice from an array flattened to a 1D vector. 
/// The slice corresponds to the current position of the iterator. The result is returned as a
/// flattened vector.
/// @param[in] in input vector
/// @return output vector (single plane)
casa::Vector<double> MultiDimArrayPlaneIter::getPlaneVector(casa::Vector<double> &in) const
{
  casa::Array<double> plane = getPlane(in);
  return plane.reform(casa::IPosition(1,plane.nelements())); 
}

/// @brief return the unique tag of the current plane
/// @details To assist caching one may need a string key which is unique for every iteration.
/// This method forms a string tag from the position vector, which can be appended to the
/// parameter name to get a unique string for every single plane.
/// @note This is an alternative way to converting sequenceNumber to string.
/// @return string tag
std::string MultiDimArrayPlaneIter::tag() const
{
  std::string res;
  const casa::IPosition curPlane = position();
  ASKAPDEBUGASSERT(curPlane.nelements() == itsShape.nelements());
  for (casa::uInt dim = 2; dim<curPlane.nelements(); ++dim) {
       // we skip all degenerate dimensions in the tag
       if (itsShape[dim]>1) {
           if (dim == 2) {
               res += ".pol";
           } else if (dim == 3) {
               res += ".chan";
           }  else {
               res += ".";
           }       
           res += utility::toString<casa::uInt>(curPlane[dim]);
       }
  }
  return res;
}

/// @brief proceed to the next iteration
/// @details A call to this method makes a step of the iterator
void MultiDimArrayPlaneIter::next()
{
  ArrayPositionIterator::next();
  itsSequenceNumber++;
}


