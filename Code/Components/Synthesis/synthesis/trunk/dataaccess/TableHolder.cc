/// @file TableHolder.h
/// @brief A class holding a table object
/// @details A simple implementation of the ITableHolder interface.
/// This class just holds a table object and returns
/// a reference to it by demand. Although, it is cheap to copy Table
/// object itself due to the reference semantics, it seems necessary to have
/// access to some derived data (e.g. some information from the feed table,
/// or data description table), which are not necessarily cheap to copy.
/// Derived classes will provide required functionality. Building derived
/// information on-demand is also expected to be implemented.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include <dataaccess/TableHolder.h>

using namespace conrad;
using namespace synthesis;

/// constructor - set the table to work with
/// @param[in] tab table to work with
TableHolder::TableHolder(const casa::Table &tab) :
              itsTable(tab) {}


/// @return a const reference to Table held by this object
const casa::Table& TableHolder::table() const throw()
{
  return itsTable;
}
