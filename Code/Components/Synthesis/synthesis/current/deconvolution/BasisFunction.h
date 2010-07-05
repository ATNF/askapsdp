/// @file
/// @brief Base class for Basis functions
/// @details Holder for basis functions used in deconvolution
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

#ifndef I_BASISFUNCTION_H
#define I_BASISFUNCTION_H
#include <casa/aips.h>
#include <boost/shared_ptr.hpp>

#include <casa/Arrays/Array.h>

#include <string>

using namespace casa;

namespace askap {
  
  namespace synthesis {
    
    /// @brief Base class for basis functions used in deconvolutio
    /// @details All the controlling is delegated to this class so that
    /// more control is possible.
    /// @ingroup Deconvolver
    
    template<typename T> class BasisFunction {
      
    public:
      typedef boost::shared_ptr<BasisFunction<T> > ShPtr;
      
      /// @brief Construct from a specified shape
      /// param[in] shape Shape of desired basis function
      BasisFunction(const IPosition shape);
      
      virtual ~BasisFunction() {};

      /// @brief Return the number of terms in the basis function
      uInt numberTerms() const {return itsShape[2];};
      
      /// @brief Return the basis function as an array
      /// @details The basis function is returned as an array
      /// of shape (nx, ny, nterms) where nx, ny are the
      /// lengths of the first two axes.
      const virtual Array<T>& basisFunction() const {return itsBasisFunction;};

      /// @brief Is the basis function orthogonal?
      virtual Bool isOrthogonal() const {return itsOrthogonal;};

      /// @brief Return the cross terms in the basis function
      /// @details If the basis functions are not orthogonal then we
      /// may need to track the cross terms. These are returned
      /// in an array of shape (nx, ny, nterms, nterms)
      const virtual Array<T>& crossTerms() const {return itsCrossTerms;};

      /// @brief Orthogonalise the basis functions using GramSchmidt.
      /// @details The basis functions are orthogonalised using the
      /// Gram-Schmidt algorithm.
      void orthogonalise();

    protected:

      IPosition itsShape;
      IPosition itsCrossTermsShape;
      Bool itsOrthogonal;
      Array<T> itsBasisFunction;
      Array<T> itsCrossTerms;
    };
    
  } // namespace synthesis
  
} // namespace askap

#include <deconvolution/BasisFunction.tcc>

#endif


