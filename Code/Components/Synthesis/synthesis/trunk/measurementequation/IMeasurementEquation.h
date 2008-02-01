/// @file
/// 
/// @brief An abstract measurement equation.
/// @details To be able to use common code regardless on the type of the
/// measurement equaiton used (i.e. ComponentEquation, ImageFFTEquation, etc)
/// we need a common ancestor of the measurement equation classes.
/// conrad::scimath::Equation is not specialised enough for this purpose.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef I_MEASUREMENT_EQUATION_H
#define I_MEASUREMENT_EQUATION_H

// own includes
#include <fitting/INormalEquations.h>
#include <dataaccess/IDataAccessor.h>
#include <dataaccess/IConstDataAccessor.h>

namespace conrad {
 
namespace synthesis {


/// @brief An abstract measurement equation.
/// @details To be able to use common code regardless on the type of the
/// measurement equaiton used (i.e. ComponentEquation, ImageFFTEquation, etc)
/// we need a common ancestor of the measurement equation classes.
/// conrad::scimath::Equation is not specialised enough for this purpose.
/// @ingroup measurementequation
struct IMeasurementEquation
{
  /// @brief empty virtual descrtuctor to make the compiler happy
  virtual ~IMeasurementEquation();

  /// @brief Predict model visibilities for one accessor (chunk).
  /// @details This prediction is done for single chunk of data only. 
  /// It seems that all measurement equations should work with accessors 
  /// rather than iterators (i.e. the iteration over chunks should be 
  /// moved to the higher level, outside this class). 
  /// @param[in] chunk a read-write accessor to work with
  virtual void predict(IDataAccessor &chunk) const = 0;

  /// @brief Calculate the normal equation for one accessor (chunk).
  /// @details This calculation is done for a single chunk of
  /// data only (one iteration).It seems that all measurement
  /// equations should work with accessors rather than iterators
  /// (i.e. the iteration over chunks should be moved to the higher
  /// level, outside this class). 
  /// @param[in] chunk a read-write accessor to work with
  /// @param[in] ne Normal equations
  virtual void calcEquations(const IConstDataAccessor &chunk,
                          conrad::scimath::INormalEquations& ne) const = 0;
};

} // namespace synthesis

} // namespace conrad

#endif // #ifndef I_MEASUREMENT_EQUATION_H

