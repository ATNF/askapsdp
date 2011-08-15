/// @file
/// @brief A high-level interface to access calibration solutions
/// @details This interface hides the database look up of the appropriate
/// calibration solution. It manages solution IDs and provides access
/// to the actual solution via ICalSolutionAccessor.
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


#ifndef I_CAL_SOLUTION_SOURCE_H
#define I_CAL_SOLUTION_SOURCE_H

// own includes
#include <calibaccess/ICalSolutionConstSource.h>
#include <calibaccess/ICalSolutionAccessor.h>

namespace askap {

namespace accessors {

/// @brief A high-level interface to access calibration solutions
/// @details This interface hides the database look up of the appropriate
/// calibration solution. It manages solution IDs and provides access
/// to the actual solution via ICalSolutionAccessor (so write operation is
/// allowed). A single solution ID refers to some gain, leakage and 
/// bandpass, although individual solutions may be obtained at different 
/// times. The read operation always delivers the "active" (i.e. most recent) solution 
/// at the given time. During write, it is up to the user to write any combination of
/// gains, leakages and bandpasses.
/// @ingroup calibaccess
struct ICalSolutionSource : virtual public ICalSolutionConstSource {

  /// @brief obtain a solution ID to store new solution
  /// @details This method provides a solution ID for a new solution. It must
  /// be called before any write operation (one needs a writable accessor to
  /// write the actual solution and to get this accessor one needs an ID).
  /// @param[in] time time stamp of the new solution in seconds since MJD of 0.
  /// @return solution ID
  virtual long newSolutionID(const double time) = 0;
  
  /// @brief obtain a writeable accessor for a given solution ID
  /// @details This method returns a shared pointer to the solution accessor, which
  /// can be used to both read the parameters and write them back. If a solution with 
  /// the given ID doesn't exist, an exception is thrown. Existing solutions with undefined 
  /// parameters are managed via validity flags of gains, leakages and bandpasses
  /// @param[in] id solution ID to access
  /// @return shared pointer to an accessor object
  virtual boost::shared_ptr<ICalSolutionAccessor> rwSolution(const long id) const = 0;
    
  /// @brief shared pointer definition
  typedef boost::shared_ptr<ICalSolutionConstSource> ShPtr;
};

} // namespace askap

} // namespace accessors 

#endif // #ifndef I_CAL_SOLUTION_SOURCE_H

