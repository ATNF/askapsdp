/// @file 
/// @brief Implementation of ITableInfoAccessor
/// @details This file contains a class, which just returns a reference
/// to SubtableInfoHolder stored in the smart pointer. This additional
/// level of wrapping is used to ship around the measurement set table
/// with its derived information.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef TABLE_INFO_ACCESSOR_H
#define TABLE_INFO_ACCESSOR_H

// casa includes
#include <tables/Tables/Table.h>

// boost includes
#include <boost/shared_ptr.hpp>

// own includes
#include <dataaccess/ITableInfoAccessor.h>
#include <dataaccess/TableManager.h>

namespace conrad {

namespace synthesis {

/// @brief Implementation of ITableInfoAccessor
/// @details This file contains a class, which just returns a reference
/// to SubtableInfoHolder stored in a smart pointer. This additional
/// level of wrapping is used to ship around the measurement set table
/// with its derived information.
/// @note see the TableManager class, which is an implementation of
/// the ISubtableInfoHolder and ITableHolder interfaces, for detailed
/// description how this bunch of classes is supposed to work together
/// @ingroup dataaccess
struct TableInfoAccessor : virtual public ITableInfoAccessor {

  /// @brief construct from a shared pointer to info holder
  /// @details This version of the constructor allows to work with any
  /// type of info holder
  /// @param tabManager a shared pointer to an table manager
  /// This pointer will be stored inside this class and used to
  /// access the table and derived information (i.e. reference semantics)
  TableInfoAccessor(const boost::shared_ptr<ITableManager const>
                    &tabManager) throw();

  /// @brief construct from a table object
  /// @details This version of the constructor creates a TableManager
  /// object for a given table and stores it as ISubtableInfoHolder 
  /// @param tab a measurement set table to work with
  /// @param useMemBuffers if true, buffers in memory will be created
  /// instead of the disk-based buffers
  TableInfoAccessor(const casa::Table &tab, bool useMemBuffers=false); 
  
  /// @return a non-const reference to Table held by this object
  virtual casa::Table& table() const throw();

  /// @return a reference to ISubtableInfoHolder
  virtual const ISubtableInfoHolder& subtableInfo() const;

  /// @return a shared pointer on infoHolder
  virtual const boost::shared_ptr<ITableManager const>&
                        getTableManager() const throw();
  
private:
  boost::shared_ptr<ITableManager const> itsTableManager;
};

} // namespace synthesis

} // namespace conrad

#endif // #define TABLE_INFO_ACCESSOR_H
