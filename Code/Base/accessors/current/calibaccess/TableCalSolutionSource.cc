/// @file
/// @brief table-based implementation of the calibration solution source
/// @details This implementation reads calibration solutions from and writes to a casa table
/// Main functionality is implemented in the corresponding TableCalSolutionFiller class.
/// This class creates an instance of the MemCalSolutionAccessor with the above mentioned filler 
/// when a writeable accessor is requested. Read-only functionality is implemented in the 
/// base class.
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

// own includes
#include <calibaccess/TableCalSolutionSource.h>
#include <calibaccess/TableCalSolutionFiller.h>
#include <calibaccess/MemCalSolutionAccessor.h>

namespace askap {

namespace accessors {

/// @brief constructor using a table defined explicitly
/// @details
/// @param[in] tab table to read the solutions from
TableCalSolutionSource::TableCalSolutionSource(const casa::Table &tab) : TableHolder(tab), TableCalSolutionConstSource(tab) {}
 
/// @brief constructor using a file name
/// @details The table is opened for writing
/// @param[in] name table file name 
TableCalSolutionSource::TableCalSolutionSource(const std::string &name) : 
   TableHolder(casa::Table(name, casa::Table::New)), TableCalSolutionConstSource(table()) {}


/// @brief obtain a solution ID to store new solution
/// @details This method provides a solution ID for a new solution. It must
/// be called before any write operation (one needs a writable accessor to
/// write the actual solution and to get this accessor one needs an ID).
/// @param[in] time time stamp of the new solution in seconds since MJD of 0.
/// @return solution ID
long TableCalSolutionSource::newSolutionID(const double time) {
   const casa::uInt newRow = table().nrow();
   // need to do something with time
   return long(newRow);
}
  
/// @brief obtain a writeable accessor for a given solution ID
/// @details This method returns a shared pointer to the solution accessor, which
/// can be used to both read the parameters and write them back. If a solution with 
/// the given ID doesn't exist, an exception is thrown. Existing solutions with undefined 
/// parameters are managed via validity flags of gains, leakages and bandpasses
/// @param[in] id solution ID to access
/// @return shared pointer to an accessor object
boost::shared_ptr<ICalSolutionAccessor> TableCalSolutionSource::rwSolution(const long id) const {
   boost::shared_ptr<TableCalSolutionFiller> filler(new TableCalSolutionFiller(table(),id));
   ASKAPDEBUGASSERT(filler);
   boost::shared_ptr<MemCalSolutionAccessor> acc(new MemCalSolutionAccessor(filler,false));
   ASKAPDEBUGASSERT(acc);
   return acc;  
}

} // namespace accessors

} // namespace askap

