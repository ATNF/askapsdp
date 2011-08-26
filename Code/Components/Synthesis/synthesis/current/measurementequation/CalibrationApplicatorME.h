/// @file
/// 
/// @brief measurement equation to apply calibration.
/// @details This is a special type of the measurement equation (i.e. it is not
/// even derived from the scimath::Equation class because it is not solvable). It
/// corrects a chunk of visibilities for calibration, leakages and bandpasses
/// obtained via the solution access interface. Unlike CalibrationMEBase and
/// PreAvgCalMEBase this class has the full measurement equation built in 
/// (essentially implemented by the solution access class returning a complete
/// jones matrix for each antenna/beam combination).
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

#ifndef CALIBRATION_APPLICATOR_ME_H
#define CALIBRATION_APPLICATOR_ME_H

// own includes
#include <measurementequation/ICalibrationApplicator.h>
#include <calibaccess/ICalSolutionConstSource.h>
#include <calibaccess/ICalSolutionConstAccessor.h>
#include <dataaccess/IDataAccessor.h>

// boost includes
#include <boost/shared_ptr.hpp>

namespace askap {

namespace synthesis {

/// @brief measurement equation to apply calibration.
/// @details This is a special type of the measurement equation (i.e. it is not
/// even derived from the scimath::Equation class because it is not solvable). It
/// corrects a chunk of visibilities for calibration, leakages and bandpasses
/// obtained via the solution access interface. Unlike CalibrationMEBase and
/// PreAvgCalMEBase this class has the full measurement equation built in 
/// (essentially implemented by the solution access class returning a complete
/// jones matrix for each antenna/beam combination). This class handles time-dependence
/// properly provided the solution source interface supports it as well.
/// @ingroup measurementequation
class CalibrationApplicatorME : virtual public ICalibrationApplicator {
public:

  /// @brief constructor 
  /// @details It initialises ME for a given solution source.
  /// @param[in] src calibration solution source to work with
  CalibrationApplicatorME(const boost::shared_ptr<accessors::ICalSolutionConstSource> &src);

  /// @brief correct model visibilities for one accessor (chunk).
  /// @details This method corrects the data in the given accessor
  /// (accessed via rwVisibility) for the calibration errors 
  /// represented by this measurement equation (i.e. an inversion of
  /// the matrix has been performed). 
  /// @param[in] chunk a read-write accessor to work with
  /// @note Need to think what to do in the inversion is unsuccessful
  /// e.g. amend flagging information? This is not yet implemented as
  /// existing accessors would throw an exception if flagging info is 
  /// changed.
  virtual void correct(accessors::IDataAccessor &chunk) const;  

protected:
  /// @brief helper method to update accessor pointer if necessary
  /// @details This method updates the accessor shared pointer if it is 
  /// uninitialised, or if it has been updated for the given time.
  /// @param[in] time timestamp (seconds since 0 MJD)
  void updateAccessor(const double time) const;
  
  /// @brief helper method to get current solution accessor
  /// @details This method returns a reference to the current solution
  /// accessor or throws an exception if it is uninitialised
  /// (this shouldn't happen if updateAccessor is called first)
  /// @return a const reference to the calibration solution accessor
  const accessors::ICalSolutionConstAccessor& calSolution() const;
  
private:
  /// @brief solution source to work with
  boost::shared_ptr<accessors::ICalSolutionConstSource> itsCalSolutionSource;
  
  /// @brief shared pointer to the current solution accessor 
  /// @details It is updated every time the time changes.
  mutable boost::shared_ptr<accessors::ICalSolutionConstAccessor> itsCalSolutionAccessor;  
  
  /// @brief solution ID corresponding to the current solution accessor
  mutable long itsCurrentSolutionID;
};

} // namespace synthesis

} // namespace askap

#endif // #ifndef CALIBRATION_APPLICATOR_ME_H


