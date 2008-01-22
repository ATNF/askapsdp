/// @file
/// 
/// @brief A generic measurement equation for calibration.
/// @details This template is designed to represent any possible measurement 
/// equation we expect to encounter in calibration. It is a result of evolution
/// of the former GainCalibrationEquation, which will probably be completely 
/// substituted by this template in the future. The common point between all
/// calibration equations is that the perfect measurement equation is passed
/// as a parmeter. It is used to populate an array of perfect visibilities
/// corresponding to metadata held by the data accessor for each row.
/// Then, the calibration effect represented by the template parameter is applied
/// (its ComplexDiffMatrix is multiplied by the ComplexDiffMatrix initialized with
/// the perfect visibilities). Using specialized templates like Product allows
/// to build a chain of calibration effects at the compile time. This template
/// implements predict/calcEquations methods and can be used with the solvers
/// in the usual way.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef CALIBRATION_ME_H
#define CALIBRATION_ME_H


// own includes
#include <fitting/GenericEquation.h>
#include <fitting/Params.h>
#include <fitting/GenericNormalEquations.h>

#include <measurementequation/MultiChunkEquation.h>
#include <dataaccess/IDataAccessor.h>
#include <fitting/ComplexDiffMatrix.h>
#include <fitting/ComplexDiff.h>
#include <fitting/Equation.h>


namespace conrad {

namespace synthesis {


/// @brief A generic measurement equation for calibration.
/// @details This template is designed to represent any possible measurement 
/// equation we expect to encounter in calibration. It is a result of evolution
/// of the former GainCalibrationEquation, which will probably be completely 
/// substituted by this template in the future. The common point between all
/// calibration equations is that the perfect measurement equation is passed
/// as a parmeter. It is used to populate an array of perfect visibilities
/// corresponding to metadata held by the data accessor for each row.
/// Then, the calibration effect represented by the template parameter is applied
/// (its ComplexDiffMatrix is multiplied by the ComplexDiffMatrix initialized with
/// the perfect visibilities). Using specialized templates like Product allows
/// to build a chain of calibration effects at the compile time. This template
/// implements predict/calcEquations methods and can be used with the solvers
/// in the usual way.
/// @ingroup measurementequation
template<typename Effect>
class CalibrationME : virtual public MultiChunkEquation,
                      virtual public conrad::scimath::GenericEquation
{
public:

  /// @brief Standard constructor using the parameters and the
  /// data iterator.
  /// @param[in] ip Parameters
  /// @param[in] idi data iterator
  /// @param[in] ime measurement equation describing perfect visibilities
  /// @note In the future, measurement equations will work with accessors
  /// only, and, therefore, the dependency on iterator will be removed
  CalibrationME(const conrad::scimath::Params& ip,
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
                         conrad::scimath::GenericNormalEquations& ne) const;
  
    
  using MultiChunkEquation::predict;
  using MultiChunkEquation::calcEquations;
  using conrad::scimath::GenericEquation::calcEquations;
       
  
  /// @brief Calculate the normal equations for the iterator
  /// @details This version iterates through all chunks of data and
  /// calls an abstract method declared in IMeasurementEquation for each 
  /// individual accessor (each iteration of the iterator)
  /// @note I hope this method is temporary, untill a proper constness of the
  /// method is restored.
  /// @param[in] ne Normal equations
  virtual void calcGenericEquations(conrad::scimath::GenericNormalEquations& ne);
  
  /// Clone this into a shared pointer
  /// @return shared pointer to a copy
  virtual ShPtr clone() const
     { return ShPtr(new CalibrationME<Effect>(*this)); }
  
  /// @brief a helper method to form a ComplexDiffMatrix for a given row
  /// @details This is the only method which depends on the template type.
  /// @param[in] acc input data accessor with the perfect visibilities
  /// @param[in] row the row number to work with
  /// @return ComplexDiffMatrix encapsulating information about measurement 
  ///         equation corresponding to the given row
  inline scimath::ComplexDiffMatrix buildComplexDiffMatrix(const IConstDataAccessor &acc,
                    casa::uInt row) const
  { return Effect::get(parameters(),acc,row) * ComplexDiffMatrix(acc.visibility().yzPlane(row)); }

      
private:
  /// @brief measurement equation giving perfect visibilities
  const IMeasurementEquation &itsPerfectVisME;  
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef CALIBRATION_ME_H
