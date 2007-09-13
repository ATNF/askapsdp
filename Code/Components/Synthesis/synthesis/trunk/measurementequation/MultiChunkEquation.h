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

#ifndef MULTI_CHUNK_EQUATION_H
#define MULTI_CHUNK_EQUATION_H

// own includes
#include <dataaccess/SharedIter.h>
#include <dataaccess/IDataIterator.h>
#include <dataaccess/IDataAccessor.h>

#include <measurementequation/IMeasurementEquation.h>

namespace conrad {

namespace synthesis {

/// @brief A measurement equation acting on an iterator.
/// @details This is a temporary class (I hope) to retain the existing 
/// interface for measurement equations, where these equations are applied
/// to all chunks (accessors) of the measurement set at once. It looks like 
/// in the future we need to redesign existing measurement equations to 
/// work with one iteration only (i.e. accessor instead of iterator). This 
/// class allows to simplify this transition, by factoring out the old
/// interface and implementing it via the new one.
/// @ingroup measurementequation
class MultiChunkEquation : virtual public IMeasurementEquation
{
public:
  /// @brief Standard constructor, which remembers data iterator.
  /// @param idi data iterator
  MultiChunkEquation(const IDataSharedIter& idi);

  /// @brief Calculate the normal equations for the iterator
  /// @details This version iterates through all chunks of data and
  /// calls an abstract method declared in IMeasurementEquation for each 
  //// individual accessor (each iteration of the iterator)
  /// @param[in] ne Normal equations
  virtual void calcEquations(conrad::scimath::NormalEquations& ne);

  /// @brief Predict model visibility for the iterator.
  /// @details This version of the predict method iterates
  /// over all chunks of data and calls an abstract method declared
  /// in IMeasurementEquation for each accessor. 
  virtual void predict();
   
  using IMeasurementEquation::predict;
  using IMeasurementEquation::calcEquations;
  
protected:
  /// @brief access to the iterator associated with this equation
  /// @return a const reference to the iterator held by this object
  const IDataSharedIter& iterator() const throw();  
private:
  /// Shared iterator for data access
  IDataSharedIter itsSharedIterator;
};

} // namespace synthesis

} // namespace conrad


#endif // #ifndef MULTI_CHUNK_EQUATION_H

