/// @file
/// 
/// @brief Interface for a measurement equation which can apply calibration
/// @details This interface defines a single 'correct' method which is
/// supposed to correct a chunk of visibilities for calibration errors. Initially
/// we had this functionality in the CalibrationMEBase, but it is worth while to
/// have a separate interface, so we could do a generic correction based on the 
/// parameters supplied by the Calibration Solution accessor. 
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

#ifndef I_CALIBRATION_APPLICATOR_H
#define I_CALIBRATION_APPLICATOR_H

// own includes
#include <dataaccess/IDataAccessor.h>

// boost includes
#include <boost/shared_ptr.hpp>

namespace askap {

namespace synthesis {

/// @brief Interface for a measurement equation which can apply calibration
/// @details This interface defines a single 'correct' method which is
/// supposed to correct a chunk of visibilities for calibration errors. Initially
/// we had this functionality in the CalibrationMEBase, but it is worth while to
/// have a separate interface, so we could do a generic correction based on the 
/// parameters supplied by the Calibration Solution accessor. 
/// @ingroup measurementequation
struct ICalibrationApplicator {
  /// @brief virtual destructor to keep the compiler happy
  virtual ~ICalibrationApplicator() {}
  
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
  virtual void correct(accessors::IDataAccessor &chunk) const = 0;  
};

} // namespace synthesis

} // namespace askap

#endif // #ifndef I_CALIBRATION_APPLICATOR_H

