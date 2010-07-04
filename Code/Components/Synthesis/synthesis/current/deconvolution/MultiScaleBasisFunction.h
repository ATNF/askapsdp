/// @file
/// @brief Holder fo multiscale functions
/// @details Holder for multiscale functions used in deconvolution
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

#ifndef I_MULTISCALEBASISFUNCTION_H
#define I_MULTISCALEBASISFUNCTION_H
#include <casa/aips.h>
#include <boost/shared_ptr.hpp>

#include <casa/Arrays/Array.h>
#include <deconvolution/BasisFunction.h>

#include <string>

using namespace casa;

namespace askap {
  
  namespace synthesis {
    
    /// @brief Holder for multiscale basis functions used in MSClean
    /// @details The basis functions used here are those used in 
    /// MSClean.
    /// @ingroup Deconvolver
    
    template<typename T>
    class MultiScaleBasisFunction : public BasisFunction<T> {
      
    public:
      typedef boost::shared_ptr<MultiScaleBasisFunction<T> > ShPtr;
      
      /// @brief Construct from a specified shape
      /// @details The scale parameter holds the set of scale sizes to be
      /// used in the basis function
      MultiScaleBasisFunction(const IPosition shape, const Vector<Float>& scales);
      
    private:
      Vector<Float> itsScales;
      T spheroidal(T nu);
    };
    
  } // namespace synthesis
  
} // namespace askap

#include <deconvolution/MultiScaleBasisFunction.tcc>

#endif


