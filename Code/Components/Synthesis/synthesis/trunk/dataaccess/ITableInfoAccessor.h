/// @file 
/// @brief An extention of ITableHolder to derived information
/// @details A class given in this file is closely related to
/// SubtableInfoHolder. This interface provides a single
/// pure virtual function, which returns a const reference to
/// SubtableInfoHolder. An additional level of wrapping is required to
/// store the class actually holding the table & its derived
/// data by pointer (managed by a smart pointer template).
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_TABLE_INFO_ACCESSOR_H
#define I_TABLE_INFO_ACCESSOR_H

// boost includes
#include <boost/shared_ptr.hpp>

// own includes
#include <dataaccess/ITableHolder.h>
#include <dataaccess/ISubtableInfoHolder.h>
#include <dataaccess/ITableManager.h>

namespace conrad {

namespace synthesis {

/// @brief An extention of ITableHolder to derived information
/// @details A class given in this file is closely related to
/// SubtableInfoHolder. This interface provides a single
/// pure virtual function, which returns a const reference to
/// SubtableInfoHolder. An additional level of wrapping is required to
/// store the class actually holding the table & its derived
/// data by pointer (managed by a smart pointer template).
struct ITableInfoAccessor : virtual public ITableHolder {
  /// @return a reference to ISubtableInfoHolder
  virtual const ISubtableInfoHolder& subtableInfo() const = 0;
  
  /// @return a shared pointer on infoHolder
  virtual const boost::shared_ptr<ITableManager const>&
                        getTableManager() const throw() = 0;

};

} // namespace synthesis

} // namespace conrad

#endif // #define I_TABLE_INFO_ACCESSOR_H
