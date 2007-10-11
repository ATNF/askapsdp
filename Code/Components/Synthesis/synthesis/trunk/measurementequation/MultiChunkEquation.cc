/// @file
/// 
/// @brief A measurement equation acting on an iterator.
/// @details This is a temporary class (I hope) to retain the existing 
/// interface for measurement equations, where these equations are applied
/// to all chunks (accessors) of the measurement set at once. It looks like 
/// in the future we need to redesign existing measurement equations to 
/// work with one iteration only (i.e. accessor instead of iterator). This 
/// class allows to simplify this transition, by factoring out the old
/// interface and implementing it via the new one.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>


// own includes
#include <measurementequation/MultiChunkEquation.h>

using namespace conrad;
using namespace conrad::synthesis;

/// @brief Standard constructor, which remembers data iterator.
/// @param idi data iterator
MultiChunkEquation::MultiChunkEquation(const IDataSharedIter& idi) :
                 itsSharedIterator(idi) {}

/// @brief Calculate the normal equations for the iterator
/// @details This version iterates through all chunks of data and
/// calls an abstract method declared in IMeasurementEquation for each 
//// individual accessor (each iteration of the iterator)
/// @param[in] ne Normal equations
void MultiChunkEquation::calcEquations(conrad::scimath::NormalEquations& ne) const
{ 
  for (itsSharedIterator.init(); itsSharedIterator.hasMore(); 
                                 itsSharedIterator.next()) {
       calcEquations(*itsSharedIterator,ne);
  }
}

/// @brief Predict model visibility for the iterator.
/// @details This version of the predict method iterates
/// over all chunks of data and calls an abstract method declared
/// in IMeasurementEquation for each accessor. 
void MultiChunkEquation::predict() const
{
  for (itsSharedIterator.init(); itsSharedIterator.hasMore(); 
                                 itsSharedIterator.next()) {
       predict(*itsSharedIterator);
  }
}
   
/// @brief access to the iterator associated with this equation
/// @return a const reference to the iterator held by this object
const IDataSharedIter& MultiChunkEquation::iterator() const throw()  
{
  return itsSharedIterator;
}

