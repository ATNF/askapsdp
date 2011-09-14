/// @file
/// @brief table-based implementation of the calibration solution source
/// @details This implementation reads calibration solutions from a casa table
/// Main functionality is implemented in the corresponding TableCalSolutionFiller class.
/// This class manages the time/row dependence and creates an instance of the 
/// MemCalSolutionAccessor with above mentioned filler when a read-only accessor is
/// requested.
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

#include <calibaccess/TableCalSolutionConstSource.h>
#include <calibaccess/TableCalSolutionFiller.h>
#include <calibaccess/MemCalSolutionAccessor.h>

namespace askap {

namespace accessors {

/// @brief constructor using a table defined explicitly
/// @details
/// @param[in] tab table to read the solutions from
TableCalSolutionConstSource::TableCalSolutionConstSource(const casa::Table &tab) : TableHolder(tab) {}
  
/// @brief constructor using a file name
/// @details The table is opened for reading and an exception is thrown if the table doesn't exist
/// @param[in] name table file name 
TableCalSolutionConstSource::TableCalSolutionConstSource(const std::string &name) : 
        TableHolder(casa::Table(name)) 
{
  ASKAPCHECK(table().nrow()>0, "The table "<<name<<" passed to TableCalSolutionConstSource is empty");
}


/// @brief obtain ID for the most recent solution
/// @return ID for the most recent solution
long TableCalSolutionConstSource::mostRecentSolution() const 
{
  // derived classes may initialise the table for writing and, therefore, it could be empty by this point
  // despite the check in the constructor
  return table().nrow() > 0 ? long(table().nrow()) - 1 : -1;
}
  
/// @brief obtain solution ID for a given time
/// @details This method looks for a solution valid at the given time
/// and returns its ID. It is equivalent to mostRecentSolution() if
/// called with a time sufficiently into the future.
/// @param[in] time time stamp in seconds since MJD of 0.
/// @return solution ID
long TableCalSolutionConstSource::solutionID(const double time) const
{
  // need to work with TIME column here
  ASKAPASSERT(time>0);
  return 0; // for now
}
  
/// @brief obtain read-only accessor for a given solution ID
/// @details This method returns a shared pointer to the solution accessor, which
/// can be used to read the parameters. If a solution with the given ID doesn't 
/// exist, an exception is thrown. Existing solutions with undefined parameters 
/// are managed via validity flags of gains, leakages and bandpasses
/// @param[in] id solution ID to read
/// @return shared pointer to an accessor object
boost::shared_ptr<ICalSolutionConstAccessor> TableCalSolutionConstSource::roSolution(const long id) const
{
  boost::shared_ptr<TableCalSolutionFiller> filler(new TableCalSolutionFiller(table(),id));
  ASKAPDEBUGASSERT(filler);
  boost::shared_ptr<MemCalSolutionAccessor> acc(new MemCalSolutionAccessor(filler,true));
  ASKAPDEBUGASSERT(acc);
  return acc;
}

} // namespace accessors

} // namespace askap

