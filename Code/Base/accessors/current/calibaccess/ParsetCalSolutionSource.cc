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

#include <calibaccess/ParsetCalSolutionSource.h>
#include <calibaccess/ParsetCalSolutionAccessor.h>

// logging stuff
#include <askap_accessors.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".calibaccess");

namespace askap {

namespace accessors {

/// @brief constructor
/// @details Creates solution source object for a given parset file
/// (whether it is for writing or reading depends on the actual methods
/// used).
/// @param[in] parset parset file name
ParsetCalSolutionSource::ParsetCalSolutionSource(const std::string &parset) :
   itsAccessor(new ParsetCalSolutionAccessor(parset)), itsFirstSolution(true) {}
  
/// @brief obtain ID for the most recent solution
/// @return ID for the most recent solution
/// @note This particular implementation doesn't support multiple
/// solutions and, therefore, always returns the same ID.
long ParsetCalSolutionSource::mostRecentSolution() const 
{
  return 0;
}
  
/// @brief obtain solution ID for a given time
/// @details This method looks for a solution valid at the given time
/// and returns its ID. It is equivalent to mostRecentSolution() if
/// called with a time sufficiently into the future.
/// @return solution ID
/// @note This particular implementation doesn't support multiple
/// solutions and, therefore, always returns the same ID.
long ParsetCalSolutionSource::solutionID(const double) const
{
  return 0; 
}
  
/// @brief obtain read-only accessor for a given solution ID
/// @details This method returns a shared pointer to the solution accessor, which
/// can be used to read the parameters. If a solution with the given ID doesn't 
/// exist, an exception is thrown. Existing solutions with undefined parameters 
/// are managed via validity flags of gains, leakages and bandpasses
/// @return shared pointer to an accessor object
/// @note This particular implementation doesn't support multiple solutions and
/// always returns the same accessor (for both reading and writing)
boost::shared_ptr<ICalSolutionConstAccessor> ParsetCalSolutionSource::roSolution(const long) const
{
  return itsAccessor;
}
  
/// @brief obtain a solution ID to store new solution
/// @details This method provides a solution ID for a new solution. It must
/// be called before any write operation (one needs a writable accessor to
/// write the actual solution and to get this accessor one needs an ID).
/// @param[in] time time stamp of the new solution in seconds since MJD of 0.
/// @return solution ID
/// @note This particular implementation always returns the same ID as it
/// doesn't hangle multiple solution. Use table-based implementation to handle
/// multiple (e.g. time-dependent) solutions
long ParsetCalSolutionSource::newSolutionID(const double time)
{
  if (itsFirstSolution) {
      ASKAPLOG_INFO_STR(logger, "About to write a new calibration solution tagged with time "<<time<<" (seconds since MJD)");
  } else {
      ASKAPLOG_WARN_STR(logger, "New calibration solution for time "<<time<<
         " (seconds since MJD); parset-based implementation doesn't support multiple solutions. Old values are going to be overwritten.");
  }
  itsFirstSolution = false;
  return 0;
}
  
/// @brief obtain a writeable accessor for a given solution ID
/// @details This method returns a shared pointer to the solution accessor, which
/// can be used to both read the parameters and write them back. If a solution with 
/// the given ID doesn't exist, an exception is thrown. Existing solutions with undefined 
/// parameters are managed via validity flags of gains, leakages and bandpasses
/// @return shared pointer to an accessor object
/// @note This particular implementation returns the same accessor regardless of the
/// chosen ID (for both reading and writing)
boost::shared_ptr<ICalSolutionAccessor> ParsetCalSolutionSource::rwSolution(const long) const
{
  return itsAccessor;
}

} // accessors

} // namespace askap

