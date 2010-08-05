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

using namespace casa;

namespace askap {
  
  namespace synthesis {
    
    template<class T>
    BasisFunction<T>::BasisFunction() : itsNumberTerms(1)
    {
    };
    
    template<class T>
    BasisFunction<T>::BasisFunction(const IPosition shape) : itsNumberTerms(1)
    {
      initialise(shape);
    };
    
    template<class T>
    void BasisFunction<T>::initialise(const IPosition shape)
    {
      ASKAPASSERT(itsNumberTerms);
      IPosition bfShape(3, shape(0), shape(1), itsNumberTerms);
      itsBasisFunction.resize(bfShape);
      itsBasisFunction.set(T(0.0));
    };
    
  } // namespace synthesis
  
} // namespace askap
