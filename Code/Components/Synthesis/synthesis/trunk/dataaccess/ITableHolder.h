/// @file ITableHolder.h
/// @brief An interface to something holding a table
/// @details A class derived from this interface holds a table and returns
/// a reference to it by demand. Although, it is cheap to copy Table
/// object itself due to reference semantics, it seems necessary to have
/// access to some derived data (e.g. some information from the feed table,
/// or data description table), which are not necessarily cheap to copy.
/// This interface is the base class in an hierarchy of classes which
/// provide required functionality. Building derived information on-demand
/// is also expected to be implemented.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_TABLE_HOLDER_H
#define I_TABLE_HOLDER_H

// casa includes
#include <tables/Tables/Table.h>

// own includes
#include <dataaccess/IHolder.h>

namespace conrad {

namespace synthesis {

/// @brief An interface to something holding a table
/// @details A class derived from this interface holds a table and returns
/// a reference to it by demand. Although, it is cheap to copy Table
/// object itself due to reference semantics, it seems necessary to have
/// access to some derived data (e.g. some information from the feed table,
/// or data description table), which are not necessarily cheap to copy.
/// This interface is the base class in an hierarchy of classes which
/// provide required functionality. Building derived information on-demand
/// is also expected to be implemented.
/// @ingroup dataaccess_tm
struct ITableHolder : virtual public IHolder {

  /// @return a non-const reference to Table held by this object
  virtual casa::Table& table() const throw() = 0;

};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef I_TABLE_HOLDER_H
