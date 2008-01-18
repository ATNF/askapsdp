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
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

// own includes
#include <fitting/ComplexDiffMatrix.h>

namespace conrad {

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
                #ifdef CONRAD_DEBUG
                CONRADCHECK(res.first->second == elem->isReal(*param), "Parameter "<<
                       *param<<" changes type (real/complex) within ComplexDiffMatrix");
                #endif // CONRAD_DEBUG
            } 
       }
  }
  itsParameterMapInvalid = false;
}

} // namespace scimath

} // namespace conrad
