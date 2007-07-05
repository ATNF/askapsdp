/// @file
/// @brief Class to manage Table and derived information
/// @details This file contains a class, which holds a table and
/// associated derived information, which is built on-demand. It
/// implements ITableHolder and ISubtableInfoHolder interfaces.
/// At this stage, the class just has a constructor defined which
/// connects individual components together. All functionality is
/// inherited from the building blocks.
/// @note There are two ways of using this class:
/// @li At the level of hierarchy where the access to such information is
///    first required, derive also (multiple inheritance is assumed) virtually
///    from ITableHolder or ISubtableInfoHolder (depending on what is required,
///    the ITableManager interface is derived from these two and provides both
///    the table and derived information access).
///    At the top level, derive also (multiple derivation is assumed) from
///    TableManager. This will implement pure virtual methods in
///    ITableHolder and ISubtableInfoHolder. This method is probably the
///    best if only one class would need access to the table. 
/// @li Hold a pointer to this class in a smart pointer template, which can
///    be passed around for all interested classes. To provide a good
///    interface and avoid multiple copies of the code managing the
///    smart pointer, ITableInfoAccessor and TableInfoAccessor classes are
///    written. Similarly to the previous method, at the level of hierarchy
///    where an access to table/derived information is required, derive
///    virtually from ITableInfoAccessor. At the top level, derive
///    from TableInfoAccessor. Its constructor accepts a smart pointer
///    to ITableManager, which TableManager is derived from.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H

// casa includes
#include <tables/Tables/Table.h>

// own includes
#include <dataaccess/ITableManager.h>
#include <dataaccess/SubtableInfoHolder.h>
#include <dataaccess/TableHolder.h>

namespace conrad {

namespace synthesis {

/// @brief Class to manage Table and derived information
/// @details This class holds a table and
/// associated derived information, which is built on-demand. It
/// implements ITableHolder and ISubtableInfoHolder interfaces.
/// At this stage, the class just has a constructor defined which
/// connects individual components together. All functionality is
/// inherited from the building blocks.
/// @note There are two ways of using this class:
/// @li At the level of hierarchy where the access to such information is
///    first required, derive also (multiple inheritance is assumed) virtually
///    from ITableHolder or ISubtableInfoHolder (depending on what is required,
///    the ITableManager interface is derived from these two and provides both
///    the table and derived information access).
///    At the top level, derive also (multiple derivation is assumed) from
///    TableManager. This will implement pure virtual methods in
///    ITableHolder and ISubtableInfoHolder. This method is probably the
///    best if only one class would need access to the table. 
/// @li Hold a pointer to this class in a smart pointer template, which can
///    be passed around for all interested classes. To provide a good
///    interface and avoid multiple copies of the code managing the
///    smart pointer, ITableInfoAccessor and TableInfoAccessor classes are
///    written. Similarly to the previous method, at the level of hierarchy
///    where an access to table/derived information is required, derive
///    virtually from ITableInfoAccessor. At the top level, derive
///    from TableInfoAccessor. Its constructor accepts a smart pointer
///    to ITableManager, which TableManager is derived from.
/// @ingroup dataaccess
struct TableManager : virtual public ITableManager,
                      virtual public TableHolder,
                      virtual public SubtableInfoHolder
{
  /// construct a table/derived info manager from the table object
  /// @param[in] tab MS table to work with
  /// @param useMemBuffers if true, buffers in memory will be created
  /// instead of the disk-based buffers
  explicit TableManager(const casa::Table &tab, bool useMemBuffers) :
           TableHolder(tab), SubtableInfoHolder(useMemBuffers) {}
};

} // namespace synthesis

} // namespace conrad

#endif // #define TABLE_MANAGER_H
