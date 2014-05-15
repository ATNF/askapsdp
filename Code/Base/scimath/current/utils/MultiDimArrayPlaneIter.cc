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
     itsPlaneShape(planeShape(shape)), itsSequenceNumber(0)
{
  ASKAPASSERT(itsShape.product()>0);
  ASKAPDEBUGASSERT(itsShape[0]>0 && itsShape[1]>0);
}

/// @brief shape of a single plane for an arbitrary cube
/// @details This method returns the shape of a single plane preserving degenerate
/// dimensions. The difference from another overloaded version of this method is 
/// that this method is static and works with an arbitrary shape of the full cube passed
/// as a parameter. The version of the method without parameters works with the cube shape
/// the object has been initialised with.
/// @param[in] shape shape of the full cube
/// @return a shape of the single plane preserving degenerate dimensions
casa::IPosition MultiDimArrayPlaneIter::planeShape(const casa::IPosition &shape)
{
  ASKAPASSERT(shape.nelements()>=2);
  casa::IPosition planeShape(shape);
  for (casa::uInt dim=2; dim<planeShape.nelements(); ++dim) {
       ASKAPDEBUGASSERT(planeShape[dim]>0);
       planeShape[dim] = 1;
  }  
  return planeShape;
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


