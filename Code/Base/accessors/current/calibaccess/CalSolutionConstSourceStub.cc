/// @file
/// @brief Stubbed implementation of the read-only calibration solution source
/// @details This is a basic stub which just returns an instance of some
/// accessor. Only read-only methods are implemented.
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

#include <calibaccess/CalSolutionConstSourceStub.h>

// logging stuff
#include <askap_accessors.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".calibaccess");

namespace askap {

namespace accessors {

/// @brief constructor
/// @details Creates solution source object for a given accessor
/// @param[in] acc shared pointer to some accessor
CalSolutionConstSourceStub::CalSolutionConstSourceStub(const boost::shared_ptr<ICalSolutionConstAccessor> &acc) :
   itsAccessor(acc) {}
  
/// @brief obtain ID for the most recent solution
/// @return ID for the most recent solution
/// @note This particular implementation doesn't support multiple
/// solutions and, therefore, always returns the same ID.
long CalSolutionConstSourceStub::mostRecentSolution() const 
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
long CalSolutionConstSourceStub::solutionID(const double) const
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
boost::shared_ptr<ICalSolutionConstAccessor> CalSolutionConstSourceStub::roSolution(const long) const
{
  return accessor();
}
  

} // accessors

} // namespace askap

