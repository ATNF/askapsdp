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

#ifndef TABLE_CAL_SOLUTION_CONST_SOURCE_H
#define TABLE_CAL_SOLUTION_CONST_SOURCE_H

// own includes
#include <calibaccess/ICalSolutionConstSource.h>
#include <dataaccess/TableHolder.h>

// casa includes
#include <tables/Tables/Table.h>

// boost includes
#include <boost/shared_ptr.hpp>

// std includes
#include <string>

namespace askap {

namespace accessors {

/// @brief table-based implementation of the calibration solution source
/// @details This implementation reads calibration solutions from a casa table
/// Main functionality is implemented in the corresponding TableCalSolutionFiller class.
/// This class manages the time/row dependence and creates an instance of the 
/// MemCalSolutionAccessor with above mentioned filler when a read-only accessor is
/// requested.
/// @ingroup calibaccess
class TableCalSolutionConstSource : virtual public ICalSolutionConstSource,
                                    virtual protected TableHolder {
public:  

  /// @brief constructor using a table defined explicitly
  /// @details
  /// @param[in] tab table to read the solutions from
  TableCalSolutionConstSource(const casa::Table &tab);
  
  /// @brief constructor using a file name
  /// @details The table is opened for reading and an exception is thrown if the table doesn't exist
  /// @param[in] name table file name 
  TableCalSolutionConstSource(const std::string &name);

  // virtual methods of the interface
  
  /// @brief obtain ID for the most recent solution
  /// @return ID for the most recent solution
  virtual long mostRecentSolution() const;
  
  /// @brief obtain solution ID for a given time
  /// @details This method looks for a solution valid at the given time
  /// and returns its ID. It is equivalent to mostRecentSolution() if
  /// called with a time sufficiently into the future.
  /// @param[in] time time stamp in seconds since MJD of 0.
  /// @return solution ID
  virtual long solutionID(const double time) const;
  
  /// @brief obtain read-only accessor for a given solution ID
  /// @details This method returns a shared pointer to the solution accessor, which
  /// can be used to read the parameters. If a solution with the given ID doesn't 
  /// exist, an exception is thrown. Existing solutions with undefined parameters 
  /// are managed via validity flags of gains, leakages and bandpasses
  /// @param[in] id solution ID to read
  /// @return shared pointer to an accessor object
  virtual boost::shared_ptr<ICalSolutionConstAccessor> roSolution(const long id) const;
  
  /// @brief shared pointer definition
  typedef boost::shared_ptr<TableCalSolutionConstSource> ShPtr;
};


} // namespace accessors
 
} // namespace askap



#endif // #ifndef TABLE_CAL_SOLUTION_CONST_SOURCE_H


