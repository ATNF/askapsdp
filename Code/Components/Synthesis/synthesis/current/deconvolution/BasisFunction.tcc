/// @file
/// @brief Basis Function for a  source
/// @details Holds basis function for a  source
/// @ingroup Deconvolver
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
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>

#include <casa/aips.h>

#include <casa/Arrays/Cube.h>

#include <deconvolution/BasisFunction.h>

using namespace casa;

namespace askap {
  
  namespace synthesis {
    
    template<class T>
    BasisFunction<T>::BasisFunction(const IPosition shape, const Bool orthogonal) : itsShape(shape), itsOrthogonal(orthogonal)
    {
      uInt ndim=itsShape.nelements();
      ASKAPASSERT(ndim==3);
      itsBasisFunction.resize(itsShape);
      itsBasisFunction.set(T(0.0));
    };
    
    // Orthogonalise using the Gram-Schmidt process. This is not optimum
    // in floating point precision and so we might want to use a better
    // algorithm one day
    // See e.g. http://planetmath.org/?op=getobj&from=objects&name=GramSchmidtOrthogonalization
    template<class T>
    void BasisFunction<T>::orthogonalise()
    {
      casa::Cube<T> v(itsBasisFunction);
      casa::Cube<T> u(itsBasisFunction.copy());
      casa::Array<T> w(u.xyPlane(0).copy());

      uInt k=v.shape()(2);

      u.xyPlane(0)=v.xyPlane(0);
      // Loop over all basis function
      for (uInt i=1;i<k;i++) {
        // Make this basis function orthogonal to all the previous
        // functions
        w.set(T(0.0));
        for (uInt j=0;j<i;j++) {
          T projection;
          projection=(sum(u.xyPlane(j)*v.xyPlane(i)))/(sum(u.xyPlane(j)*u.xyPlane(j)));
          w=w+projection*u.xyPlane(j);
        }
        u.xyPlane(i)=v.xyPlane(i)-w;
      }
      // Now Normalise to unit integral
      // Otherwise the normalisation should be v.xyPlane(j)=u.xyPlane(j)/sqrt(sum(u.xyPlane(j)*u.xyPlane(j)));
      for (uInt j=0;j<k;j++) {
        v.xyPlane(j)=u.xyPlane(j)/sum(u.xyPlane(j));
      }
      itsOrthogonal=True;
    }
  } // namespace synthesis
  
} // namespace askap
