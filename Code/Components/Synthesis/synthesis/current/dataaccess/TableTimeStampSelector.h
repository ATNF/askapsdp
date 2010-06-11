/// @file TableTimeStampSelector.h
/// @brief Generalized selection on epoch in the table-based case
/// @details This class representing a selection of visibility
///          data on some time interval. It implements the abstract
///          method updating table expression node via a new
///          abstract method, which just return start and stop
///          times as Double in the same frame/units as the TIME
///          column in the table. These two methods are specified
///          in the derived classes.
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
///
#ifndef TABLE_TIME_STAMP_SELECTOR_H
#define TABLE_TIME_STAMP_SELECTOR_H

// casa
#include <tables/Tables/Table.h>
#include <measures/Measures/MEpoch.h>

// std includes
#include <utility>
#include <string>

// own includes
#include <dataaccess/TableMeasureFieldSelector.h>
#include <dataaccess/ITableHolder.h>
#include <dataaccess/TimeDependentSubtable.h>

namespace askap {

namespace synthesis {

/// @brief Generalized selection on epoch in the table-based case
/// @details This class representing a selection of visibility
///          data on some time interval. It implements the abstract
///          method updating table expression node via a new
///          abstract method, which just return start and stop
///          times as Double in the same frame/units as the TIME
///          column in the table. These two methods are specified
///          in the derived classes.
/// @ingroup dataaccess_tab
class TableTimeStampSelector : public TableMeasureFieldSelector,
                               virtual protected ITableHolder,
                               virtual protected TimeDependentSubtable
{
public:
  
   /// main method, updates table expression node to narrow down the selection
   ///
   /// @param tex a reference to table expression to use
   virtual void updateTableExpression(casa::TableExprNode &tex) const;

protected:
  
   /// @brief This method has to be overriden in derived classes.
   /// @details According to the interface, the data converter is
   /// not available inside the chooseTimeRange method. It only becomes
   /// available when the iterator is created. Therefore, the processing
   /// of the time selection has to be deferred until the converter is known.
   /// This method can be overridden in two different ways depending on whether
   /// the selection is done using time as Double or as MVTime.  
   /// @return start and stop times of the interval to be selected (as
   ///         an std::pair, start is first, stop is second)
   virtual std::pair<casa::MEpoch, casa::MEpoch>
           getStartAndStop() const = 0;       
};

} // namespace askap

} // namespace askap

#endif /// #ifndef TABLE_TIME_STAMP_SELECTOR_H
