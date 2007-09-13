/// @file
/// 
/// @brief A measurement equation describing antenna gains.
/// @details This measurement equation just multiplies by a gain matrix
/// visibilities produced by another measurement equation. It also generates
/// normal equations, which allow to solve for unknowns in the gain matrix.
/// 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef GAIN_CALIBRATION_EQUATION_H
#define GAIN_CALIBRATION_EQUATION_H

// own includes
#include <fitting/Equation.h>
#include <fitting/Params.h>
#include <fitting/NormalEquations.h>

#include <measurementequation/MultiChunkEquation.h>
#include <dataaccess/IDataAccessor.h>

namespace conrad {

namespace synthesis {

/// @brief A measurement equation describing antenna gains.
/// @details This measurement equation just multiplies by a gain matrix
/// visibilities produced by another measurement equation. It also generates
/// normal equations, which allow to solve for unknowns in the gain matrix.
/// @ingroup measurementequation
class GainCalibrationEquation : virtual public MultiChunkEquation,
                                virtual public conrad::scimath::Equation
{
public:

  /// @brief Standard constructor using the parameters and the
  /// data iterator.
  /// @param[in] ip Parameters
  /// @param[in] idi data iterator
  /// @param[in] ime measurement equation describing perfect visibilities
  /// @note In the future, measurement equations will work with accessors
  /// only, and, therefore, the dependency on iterator will be removed
  GainCalibrationEquation(const conrad::scimath::Params& ip,
          const IDataSharedIter& idi, const IMeasurementEquation &ime);
  
  /// @brief Predict model visibilities for one accessor (chunk).
  /// @details This version of the predict method works with
  /// a single chunk of data only. It seems that all measurement
  /// equations should work with accessors rather than iterators
  /// (i.e. the iteration over chunks should be moved to the higher
  /// level, outside this class). In the future, I expect that
  /// predict() without parameters will be deprecated.
  /// @param[in] chunk a read-write accessor to work with
  virtual void predict(IDataAccessor &chunk) const;

  /// @brief Calculate the normal equation for one accessor (chunk).
  /// @details This version of the method works on a single chunk of
  /// data only (one iteration).It seems that all measurement
  /// equations should work with accessors rather than iterators
  /// (i.e. the iteration over chunks should be moved to the higher
  /// level, outside this class). In the future, I expect that
  /// the variant of the method without parameters will be deprecated.
  /// @param[in] chunk a read-write accessor to work with
  /// @param[in] ne Normal equations
  virtual void calcEquations(const IConstDataAccessor &chunk,
                                   conrad::scimath::NormalEquations& ne) const;
  
  using MultiChunkEquation::predict;
  using MultiChunkEquation::calcEquations;
  
private:
  /// @brief measurement equation giving perfect visibilities
  const IMeasurementEquation &itsPerfectVisME;   
};

} // namespace synthesis

} // namespace conrad

#endif // #ifndef GAIN_CALIBRATION_EQUATION_H
