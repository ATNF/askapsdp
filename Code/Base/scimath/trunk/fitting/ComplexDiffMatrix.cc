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
/// about. The flag itsParameterSetInvalid is reset to false at the end.
/// @note This method is conseptually constant as it works with the cache
/// only.
void ComplexDiffMatrix::buildParameterSet() const
{
  itsParameterSet.clear(); // start from scratch
  
  // iterate over elements in a flattened storage, actual shape doesn't matter
  // as we're building a union of individual sets anyway.
  for (const_iterator elem=begin(); elem!=end(); ++elem) {
       itsParameterSet.insert(elem->begin(),elem->end());     
  }
  itsParameterSetInvalid = false;
}

} // namespace scimath

} // namespace conrad
