/// @file
/// 
/// @brief A measurement equation, which is a sum of two measurement
/// equations.
/// @details For simulation it is necessary to be able to add noise 
/// to the simulated visibilities. One way of doing this is to write
/// a special measurement equation which predict noise and use a 
/// composite equation when a prediction must be made. Such an equation
/// can't be solved with a regular solver (due to a stochastic nature
/// of the problem statistical estimators are needed), but prediction
/// would work. Another application of this class is a composite imaging
/// equation where the model is composed from an image and a list of 
/// components. If there are many other additive effects to be implemented
/// and/or solution for parameters is required, the measurement equation
/// corresponding to the random visibility noise generator can be reorganized
/// into a template + individual effects in a similar way to that how 
/// CalibrationME template is written. 
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef SUM_OF_TWO_ME_S_H
#define SUM_OF_TWO_ME_S_H

// own includes
#include <measurementequation/IMeasurementEquation.h>
#include <measurementequation/MultiChunkEquation.h>
// just to be able to benefit from polymorphism
#include <fitting/Equation.h>
#include <fitting/INormalEquations.h>


// boost includes
#include <boost/shared_ptr.hpp>

namespace askap {

namespace synthesis {


/// @brief A measurement equation, which is a sum of two measurement
/// equations.
/// @details For simulation it is necessary to be able to add noise 
/// to the simulated visibilities. One way of doing this is to write
/// a special measurement equation which predict noise and use a 
/// composite equation when a prediction must be made. Such an equation
/// can't be solved with a regular solver (due to a stochastic nature
/// of the problem statistical estimators are needed), but prediction
/// would work. Another application of this class is a composite imaging
/// equation where the model is composed from an image and a list of 
/// components. If there are many other additive effects to be implemented
/// and/or solution for parameters is required, the measurement equation
/// corresponding to the random visibility noise generator can be reorganized
/// into a template + individual effects in a similar way to that how 
/// CalibrationME template is written. 
/// @note The dependence on MultiChunkEquation and an extra parameter
/// in the constructor are added temporarily while we still have 
/// iterator-based interface in the imaging code. I expect this to be
/// removed some time in the future.
/// @ingroup measurementequation
struct SumOfTwoMEs : virtual public MultiChunkEquation
{
  /// @brief Constructor   
  /// @details Creates a new composite measurement equation equivalent
  /// to a sum of the given equations. Equations passed as parameters 
  /// are not changed.
  /// @param[in] first a shared pointer to the first equation
  /// @param[in] second a shared pointer to the second equation
  /// @param[in] it iterator to work with (temporary)
  SumOfTwoMEs(const boost::shared_ptr<IMeasurementEquation const> &first,
              const boost::shared_ptr<IMeasurementEquation const> &second,
              const IDataSharedIter &it);
  
  /// @brief Predict model visibilities for one accessor (chunk).
  /// @details This prediction is done for single chunk of data only. 
  /// It seems that all measurement equations should work with accessors 
  /// rather than iterators (i.e. the iteration over chunks should be 
  /// moved to the higher level, outside this class). 
  /// @param[in] chunk a read-write accessor to work with
  virtual void predict(IDataAccessor &chunk) const;

  /// @brief Calculate the normal equation for one accessor (chunk).
  /// @details This calculation is done for a single chunk of
  /// data only (one iteration). It seems that all measurement
  /// equations should work with accessors rather than iterators
  /// (i.e. the iteration over chunks should be moved to the higher
  /// level, outside this class). 
  /// @note This method will work correctly only if two parts of the
  /// equation are completely independent. If there is a common 
  /// parameter for both parts, normal equations on that parameter will be
  /// wrong because the cross terms are omitted. This class is currently
  /// seen to be used for simulations (where only predict method is used),
  /// therefore it is not an issue. However, if a proper functionality is 
  /// required, the only way to achieve it is to use a similar approach
  /// to CalibrationME template and plug in effects.
  /// @param[in] chunk a read-write accessor to work with
  /// @param[in] ne Normal equations
  virtual void calcEquations(const IConstDataAccessor &chunk,
                          askap::scimath::INormalEquations& ne) const;

  /// Clone this into a shared pointer
  /// @return shared pointer to a copy
  virtual scimath::Equation::ShPtr clone() const;

  /// Predict the data from the parameters.
  virtual void predict() const;

  /// Calculate the normal equations for the given data and parameters
  /// @param ne Normal equations to be filled
  virtual void calcEquations(scimath::INormalEquations& ne) const;

private:
  /// @brief first measurement equation
  boost::shared_ptr<IMeasurementEquation const>  itsFirstME;
  
  /// @brief second measurement equation
  boost::shared_ptr<IMeasurementEquation const>  itsSecondME;
};

} // namespace synthesis

} // namespace askap

#endif // #ifndef SUM_OF_TWO_ME_S_H

