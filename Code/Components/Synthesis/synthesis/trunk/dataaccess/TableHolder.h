/// @file 
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

#ifndef TABLE_HOLDER_H
#define TABLE_HOLDER_H

// own includes
#include <dataaccess/ITableHolder.h>

namespace conrad {

namespace synthesis {

/// @brief A class holding a table object
/// @details A simple implementation of the ITableHolder interface.
/// This class just holds a table object and returns
/// a reference to it by demand. Although, it is cheap to copy Table
/// object itself due to the reference semantics, it seems necessary to have
/// access to some derived data (e.g. some information from the feed table,
/// or data description table), which are not necessarily cheap to copy.
/// Derived classes will provide required functionality. Building derived
/// information on-demand is also expected to be implemented.
struct TableHolder : virtual public ITableHolder {

  /// constructor - set the table to work with
  /// @param[in] tab table to work with
  TableHolder(const casa::Table &tab);

  /// @return a non-const reference to Table held by this object
  virtual casa::Table& table() const throw();

private:
  /// managed table object
  mutable casa::Table itsTable;  
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef TABLE_HOLDER_H
