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

#include <dataaccess/TableInfoAccessor.h>
#include <dataaccess/TableManager.h>
#include <conrad/ConradError.h>

using namespace conrad;
using namespace synthesis;


/// @brief construct from a shared pointer to info holder
/// @details This version of the constructor allows to work with any
/// type of info holder
/// @param infoHolder a shared pointer to an instance of info holder.
/// This pointer will be stored inside this class and used to
/// access the table and derived information (i.e. reference semantics)
TableInfoAccessor::TableInfoAccessor(const
          boost::shared_ptr<ISubtableInfoHolder const> &infoHolder)
	  throw() : itsInfoHolder(infoHolder) {}

/// @brief construct from a table object
/// @details This version of the constructor creates a TableManager
/// object for a given table and stores it as ISubtableInfoHolder 
/// @param tab a measurement set table to work with
TableInfoAccessor::TableInfoAccessor(const casa::Table &tab) :
       itsInfoHolder(new TableManager(tab)) {}


/// @return a const reference to Table held by this object
const casa::Table& TableInfoAccessor::table() const throw()
{
  CONRADDEBUGASSERT(itsInfoHolder);
  return itsInfoHolder->table();
}

/// @return a reference to ISubtableInfoHolder
const ISubtableInfoHolder& TableInfoAccessor::subtableInfo() const
{
  CONRADDEBUGASSERT(itsInfoHolder);
  return *itsInfoHolder;
}

/// @return a shared pointer on infoHolder
const boost::shared_ptr<ISubtableInfoHolder const>& 
               TableInfoAccessor::getTableManager() const throw()
{
  return itsInfoHolder;
}
