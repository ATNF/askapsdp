/// @file
/// 
/// @brief A matrix of ComplexDiff classes
/// @details The calibration code constructs normal equations for each
/// row of the data accessor, i.e. a matrix with dimensions nchan x npol.
/// When a design matrix is constructed, all elements of this matrix are
/// treated independently. However, it is better to retain a basic matrix
/// algebra to ensure the code is clear. This class also treats well a
/// possible degenerate dimension (polarisation). Theoretically, 
/// casa::Matrix<ComplexDiff> could be used instead of this class. However,
/// having a separate class allows, in principle, to handle maps of the 
/// parameters at the matrix level and don't duplicate the map search 
/// unnecessarily. Such functionality is in the future plans, but it is
/// hidden behind the interface of this class.
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

// own includes
#include <fitting/ComplexDiffMatrix.h>
#include <algorithm>

namespace askap {

namespace scimath {

/// @brief build the list of all known parameters
/// @details This method makes the cache up to date. It iterates through
/// all elements of this matrix and builds a set of parameters they know
/// about. The flag itsParameterMapInvalid is reset to false at the end.
/// @note This method is conseptually constant as it works with the cache
/// only.
void ComplexDiffMatrix::buildParameterMap() const
{
  itsParameters.clear(); // start from scratch
  
  // iterate over elements in a flattened storage, actual shape doesn't matter
  // as we're building a union of individual sets anyway.
  for (const_iterator elem=begin(); elem!=end(); ++elem) {
       for (ComplexDiff::parameter_iterator param = elem->begin();
            param != elem->end(); ++param) {
            
            std::pair<std::map<std::string, bool>::iterator, bool> res = 
                 itsParameters.insert(std::pair<std::string, bool>(*param, true)); 
            
            if (res.second) {
                // the parameter was actually insterted, so it is new
                // we need to corrent real vs. complex flag.
                res.first->second = elem->isReal(*param);
            }  else  {
                // parameter already exists, check conformance in the debug mode
                #ifdef ASKAP_DEBUG
                ASKAPCHECK(res.first->second == elem->isReal(*param), "Parameter "<<
                       *param<<" changes type (real/complex) within ComplexDiffMatrix");
                #endif // ASKAP_DEBUG
            } 
       }
  }
  itsParameterMapInvalid = false;
}

/// @brief set all element to a given value
/// @param[in] val value
void ComplexDiffMatrix::set(const ComplexDiff &val)
{
  ASKAPDEBUGASSERT(itsElements.size() > 0);
  std::fill(itsElements.begin(),itsElements.end(),val);
  itsParameterMapInvalid = true;
}

} // namespace scimath

} // namespace askap
