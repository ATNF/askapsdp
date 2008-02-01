/// @file
/// 
/// @brief Base class for generic measurement equation for calibration.
/// @details This is a base class for a template designed to represent any 
/// possible measurement equation we expect to encounter in calibration. 
/// It is a result of evolution of the former GainCalibrationEquation, which 
/// will probably be completely substituted by this template in the future. 
/// See CalibrationME template for more details. This class contains all
/// functionality, which doesn't depend on the template parameter.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef CALIBRATION_ME_BASE_H
#define CALIBRATION_ME_BASE_H


// own includes
#include <fitting/GenericEquation.h>
#include <fitting/Params.h>
#include <fitting/GenericNormalEquations.h>

#include <measurementequation/GenericMultiChunkEquation.h>
#include <dataaccess/IDataAccessor.h>
#include <fitting/ComplexDiffMatrix.h>
#include <fitting/ComplexDiff.h>
#include <fitting/Equation.h>


namespace conrad {

namespace synthesis {


/// @brief Base class for generic measurement equation for calibration.
/// @details This is a base class for a template designed to represent any 
/// possible measurement equation we expect to encounter in calibration. 
/// It is a result of evolution of the former GainCalibrationEquation, which 
/// will probably be completely substituted by this template in the future. 
/// See CalibrationME template for more details. This class contains all
/// functionality, which doesn't depend on the template parameter.
/// @ingroup measurementequation
class CalibrationMEBase : public GenericMultiChunkEquation
{
public:

  /// @brief Standard constructor using the parameters and the
  /// data iterator.
  /// @param[in] ip Parameters
  /// @param[in] idi data iterator
  /// @param[in] ime measurement equation describing perfect visibilities
  /// @note In the future, measurement equations will work with accessors
  /// only, and, therefore, the dependency on iterator will be removed
  CalibrationMEBase(const conrad::scimath::Params& ip,
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
       
 
protected:  
  /// @brief a helper method to form a ComplexDiffMatrix for a given row
  /// @details This is the only method which depends on the template type.
  /// Therefore in this class it is just declared pure virtual. This method
  /// is used on the most outer level of the measurement equation chain. Therefore,
  /// making it virtual doesn't cause problems with the compile time building of
  /// the measurement equation.
  /// @param[in] acc input data accessor with the perfect visibilities
  /// @param[in] row the row number to work with
  /// @return ComplexDiffMatrix encapsulating information about measurement 
  ///         equation corresponding to the given row
  virtual scimath::ComplexDiffMatrix buildComplexDiffMatrix(const IConstDataAccessor &acc,
                    casa::uInt row) const = 0;
      
private:
  /// @brief measurement equation giving perfect visibilities
  const IMeasurementEquation &itsPerfectVisME;  
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef CALIBRATION_ME_BASE_H
