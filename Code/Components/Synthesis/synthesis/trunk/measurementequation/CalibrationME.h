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
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef CALIBRATION_ME_H
#define CALIBRATION_ME_H

// casa includes
#include <casa/Arrays/MatrixMath.h>

// own includes
#include <fitting/GenericEquation.h>
#include <fitting/Params.h>
#include <fitting/GenericNormalEquations.h>

#include <dataaccess/IConstDataAccessor.h>
#include <fitting/ComplexDiffMatrix.h>
#include <fitting/ComplexDiff.h>
#include <measurementequation/CalibrationMEBase.h>


namespace askap {

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
class CalibrationME : public CalibrationMEBase 
{
public:

  /// @brief Standard constructor using the parameters and the
  /// data iterator.
  /// @param[in] ip Parameters
  /// @param[in] idi data iterator
  /// @param[in] ime measurement equation describing perfect visibilities
  /// @note In the future, measurement equations will work with accessors
  /// only, and, therefore, the dependency on iterator will be removed
  CalibrationME(const askap::scimath::Params& ip,
          const IDataSharedIter& idi, 
          const boost::shared_ptr<IMeasurementEquation const> &ime) :
            scimath::Equation(ip), MultiChunkEquation(idi), askap::scimath::GenericEquation(ip),
            CalibrationMEBase(parameters(), idi, ime), itsEffect(parameters()) {}
  
  /// Clone this into a shared pointer
  /// @return shared pointer to a copy
  virtual ShPtr clone() const
     { return ShPtr(new CalibrationME<Effect>(*this)); }

protected:  
  /// @brief a helper method to form a ComplexDiffMatrix for a given row
  /// @details This is the only method which depends on the template type.
  /// @param[in] acc input data accessor with the perfect visibilities
  /// @param[in] row the row number to work with
  /// @return ComplexDiffMatrix encapsulating information about measurement 
  ///         equation corresponding to the given row
  virtual scimath::ComplexDiffMatrix buildComplexDiffMatrix(const IConstDataAccessor &acc,
                    casa::uInt row) const
      {   return itsEffect.get(acc,row); }

private:
   /// @brief effectively a measurement equation
   /// @details The measurement equation is assembled at compile time. It is
   /// initialized with the reference to paramters in the constructor of this
   /// class and then used inside buildComplexDiffMatrix method.
   Effect itsEffect;
};


} // namespace synthesis

} // namespace askap

#endif // #ifndef CALIBRATION_ME_H
