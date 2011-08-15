/// @file
/// @brief A high-level interface to access calibration solutions
/// @details This interface hides the database look up of the appropriate
/// calibration solution. It manages solution IDs and provides access
/// to the actual solution via ICalSolutionConstAccessor.
///
/// @copyright (c) 2011 CSIRO
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
/// @author Max Voronkov <Maxim.Voronkov@csiro.au>

#ifndef I_CAL_SOLUTION_CONST_SOURCE_H
#define I_CAL_SOLUTION_CONST_SOURCE_H

#include <calibaccess/ICalSolutionConstAccessor.h>

namespace askap {

namespace accessors {

/// @brief A high-level interface to access calibration solutions
/// @details This interface hides the database look up of the appropriate
/// calibration solution. It manages solution IDs and provides access
/// to the actual solution via ICalSolutionConstAccessor. A single
/// solution ID refers to some gain, leakage and bandpass, although 
/// individual solutions may be obtained at different times. The read
/// operation always delivers the "active" (i.e. most recent) solution 
/// at the given time.
/// @ingroup calibaccess
struct ICalSolutionConstSource {

  /// @brief virtual destructor to keep the compiler happy
  virtual ~ICalSolutionConstSource();
  
  // virtual methods to be overridden in implementations
  
  /// @brief obtain ID for the most recent solution
  /// @return ID for the most recent solution
  virtual long mostRecentSolution() const = 0;
  
  /// @brief obtain solution ID for a given time
  /// @details This method looks for a solution valid at the given time
  /// and returns its ID. It is equivalent to mostRecentSolution() if
  /// called with a time sufficiently into the future.
  /// @param[in] time time stamp in seconds since MJD of 0.
  /// @return solution ID
  virtual long solutionID(const double time) const = 0;
  
  /// @brief obtain read-only accessor for a given solution ID
  /// @details This method returns a shared pointer to the solution accessor, which
  /// can be used to read the parameters. If a solution with the given ID doesn't 
  /// exist, an exception is thrown. Existing solutions with undefined parameters 
  /// are managed via validity flags of gains, leakages and bandpasses
  /// @param[in] id solution ID to read
  /// @return shared pointer to an accessor object
  virtual boost::shared_ptr<ICalSolutionConstAccessor> roSolution(const long id) const = 0;
  
  /// @brief shared pointer definition
  typedef boost::shared_ptr<ICalSolutionConstSource> ShPtr;
};

} // namespace accessors

} // namespace askap

#endif // #ifndef I_CAL_SOLUTION_CONST_SOURCE_H

