/// @file
/// 
/// @brief A measurement equation, which does nothing.
/// @details The current calibration class requires a perfect measurement
/// equation. This class has been written to be able to use the same code 
/// for both applying a calibration and solving for parameters. It is 
/// a void measurement equation in the sense that it does nothing to the 
/// data or normal equations given to it.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef VOID_MEASUREMENT_EQUATION_H
#define VOID_MEASUREMENT_EQUATION_H

#include <measurementequation/IMeasurementEquation.h>

namespace conrad {

namespace synthesis {

/// @brief A measurement equation, which does nothing.
/// @details The current calibration class requires a perfect measurement
/// equation. This class has been written to be able to use the same code 
/// for both applying a calibration and solving for parameters. It is 
/// a void measurement equation in the sense that it does nothing to the 
/// data or normal equations given to it.
/// @ingroup measurementequation
struct VoidMeasurementEquation : public IMeasurementEquation
{
  /// @brief Predict model visibilities for one accessor (chunk).
  /// @details This prediction is done for single chunk of data only. 
  /// It seems that all measurement equations should work with accessors 
  /// rather than iterators (i.e. the iteration over chunks should be 
  /// moved to the higher level, outside this class). 
  /// @param[in] chunk a read-write accessor to work with
  virtual void predict(IDataAccessor &chunk) const;

  /// @brief Calculate the normal equation for one accessor (chunk).
  /// @details This calculation is done for a single chunk of
  /// data only (one iteration).It seems that all measurement
  /// equations should work with accessors rather than iterators
  /// (i.e. the iteration over chunks should be moved to the higher
  /// level, outside this class). 
  /// @param[in] chunk a read-write accessor to work with
  /// @param[in] ne Normal equations
  virtual void calcEquations(const IConstDataAccessor &chunk,
                          conrad::scimath::INormalEquations& ne) const;
};

} // namespace synthesis

} // namespace conrad

#endif // #ifndef VOID_MEASUREMENT_EQUATION_H