/// @file TableTimeStampSelector.h
///
/// TableTimeStampSelector: Class representing a selection of visibility
///                data on some time interval. It implements the abstract
///                method updating table expression node via a new
///                abstract method, which just return start and stop
///                times as Double in the same frame/units as the TIME
///                column in the table. These two methods are specified
///                in the derived classes.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef TABLE_TIME_STAMP_SELECTOR_H
#define TABLE_TIME_STAMP_SELECTOR_H

/// std includes
#include <utility>
#include <string>

/// own includes
#include <dataaccess/TableMeasureFieldSelector.h>

namespace conrad {

namespace synthesis {

class TableTimeStampSelector : public TableMeasureFieldSelector
{
public:
   /// main method, updates table expression node to narrow down the selection
   ///
   /// @param tex a reference to table expression to use
   virtual void updateTableExpression(casa::TableExprNode &tex) const;

protected:
  
   /// @return start and stop times of the interval to be selected (as
   ///         an std::pair, start is first, stop is second)
   /// @param[in] frame a name of the native reference frame for a table
   /// @param[in] units native units used in the table
   virtual std::pair<casa::Double, casa::Double> 
           getStartAndStop(const std::string &frame,
                           const std::string &units) const = 0;
};

} // namespace conrad

} // namespace conrad

#endif /// #ifndef TABLE_TIME_STAMP_SELECTOR_H
