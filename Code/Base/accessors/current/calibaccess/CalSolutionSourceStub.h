/// @file
/// @brief Stubbed implementation of the calibration solution source
/// @details This is a basic stub which just returns an instance of some
/// accessor.
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


#ifndef CAL_SOLUTION_SOURCE_STUB_H
#define CAL_SOLUTION_SOURCE_STUB_H

#include <calibaccess/ICalSolutionSource.h>
#include <calibaccess/ICalSolutionAccessor.h>

namespace askap {

namespace accessors {

/// @brief Stubbed implementation of the calibration solution source
/// @details This is a basic stub which just returns an instance of some
/// accessor.
/// @ingroup calibaccess
class CalSolutionSourceStub : virtual public ICalSolutionSource {
public:

  /// @brief constructor
  /// @details Creates solution source object for a given accessor
  /// @param[in] acc shared pointer to some accessor
  explicit CalSolutionSourceStub(const boost::shared_ptr<ICalSolutionAccessor> &acc);
  
  /// @brief obtain ID for the most recent solution
  /// @return ID for the most recent solution
  /// @note This particular implementation doesn't support multiple
  /// solutions and, therefore, always returns the same ID.
  virtual long mostRecentSolution() const;
  
  /// @brief obtain solution ID for a given time
  /// @details This method looks for a solution valid at the given time
  /// and returns its ID. It is equivalent to mostRecentSolution() if
  /// called with a time sufficiently into the future.
  /// @param[in] time time stamp in seconds since MJD of 0.
  /// @return solution ID
  /// @note This particular implementation doesn't support multiple
  /// solutions and, therefore, always returns the same ID.
  virtual long solutionID(const double time) const;
  
  /// @brief obtain read-only accessor for a given solution ID
  /// @details This method returns a shared pointer to the solution accessor, which
  /// can be used to read the parameters. If a solution with the given ID doesn't 
  /// exist, an exception is thrown. Existing solutions with undefined parameters 
  /// are managed via validity flags of gains, leakages and bandpasses
  /// @param[in] id solution ID to read
  /// @return shared pointer to an accessor object
  /// @note This particular implementation doesn't support multiple solutions and
  /// always returns the same accessor (for both reading and writing)
  virtual boost::shared_ptr<ICalSolutionConstAccessor> roSolution(const long id) const;
  

  /// @brief obtain a solution ID to store new solution
  /// @details This method provides a solution ID for a new solution. It must
  /// be called before any write operation (one needs a writable accessor to
  /// write the actual solution and to get this accessor one needs an ID).
  /// @param[in] time time stamp of the new solution in seconds since MJD of 0.
  /// @return solution ID
  /// @note This particular implementation always returns the same ID as it
  /// doesn't hangle multiple solution. Use table-based implementation to handle
  /// multiple (e.g. time-dependent) solutions
  virtual long newSolutionID(const double time);
  
  /// @brief obtain a writeable accessor for a given solution ID
  /// @details This method returns a shared pointer to the solution accessor, which
  /// can be used to both read the parameters and write them back. If a solution with 
  /// the given ID doesn't exist, an exception is thrown. Existing solutions with undefined 
  /// parameters are managed via validity flags of gains, leakages and bandpasses
  /// @param[in] id solution ID to access
  /// @return shared pointer to an accessor object
  /// @note This particular implementation returns the same accessor regardless of the
  /// chosen ID (for both reading and writing)
  virtual boost::shared_ptr<ICalSolutionAccessor> rwSolution(const long id) const;
    
private:
  /// @brief accessor doing actual work
  boost::shared_ptr<ICalSolutionAccessor> itsAccessor;
  
  /// @brief helper flag that at least no solution has been written before
  /// @details We use this to give a warning which might help us in the future when
  /// ccalibrator supports time-dependent solutions and stubbed implementation
  /// (which doesn't not support time-dependent behavior) is chosen by mistake.
  bool itsFirstSolution;
};


} // namespace accessors

} // namespace askap


#endif // #ifndef CAL_SOLUTION_SOURCE_STUB_H


