/// @file
/// @brief Parset file-based implementation of the calibration solution source
/// @details This implementation is to be used with pre-existing code writing/reading
/// the parset directly and with a number of tests. It is just to convert the legacy code.
/// There is only one implementation of this class which is used for both reading and writing.
/// Main functionality is implemented in the corresponding ParsetCalSolutionAccessor class.
/// This class just creates an instance of the accessor and manages it.
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


#ifndef PARSET_CAL_SOLUTION_SOURCE_H
#define PARSET_CAL_SOLUTION_SOURCE_H

#include <calibaccess/ICalSolutionSource.h>
#include <calibaccess/ParsetCalSolutionAccessor.h>

namespace askap {

namespace accessors {

/// @brief Parset file-based implementation of the calibration solution source
/// @details This implementation is to be used with pre-existing code writing/reading
/// the parset directly and with a number of tests. It is just to convert the legacy code.
/// There is only one implementation of this class which is used for both reading and writing.
/// Main functionality is implemented in the corresponding ParsetCalSolutionAccessor class.
/// This class just creates an instance of the accessor and manages it.
/// @ingroup calibaccess
class ParsetCalSolutionSource : virtual public ICalSolutionSource {
public:

  /// @brief constructor
  /// @details Creates solution source object for a given parset file
  /// (whether it is for writing or reading depends on the actual methods
  /// used).
  /// @param[in] parset parset file name
  explicit ParsetCalSolutionSource(const std::string &parset);
  
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
    
  /// @brief shared pointer definition
  typedef boost::shared_ptr<ParsetCalSolutionSource> ShPtr;
private:
  /// @brief accessor doing actual work
  boost::shared_ptr<ParsetCalSolutionAccessor> itsAccessor;
  
  /// @brief helper flag that at least no solution has been written before
  /// @details We use this to give a warning which might help us in the future when
  /// ccalibrators supports time-dependent solutions and parset-based implementation
  /// (which doesn't not support time-dependent behavior) is chosen by mistake.
  bool itsFirstSolution;
};


} // namespace accessors

} // namespace askap


#endif // #ifndef PARSET_CAL_SOLUTION_SOURCE_H


