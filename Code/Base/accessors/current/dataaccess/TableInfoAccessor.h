/// @file 
/// @brief Implementation of ITableInfoAccessor
/// @details This file contains a class, which just returns a reference
/// to SubtableInfoHolder stored in the smart pointer. This additional
/// level of wrapping is used to ship around the measurement set table
/// with its derived information.
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

#ifndef TABLE_INFO_ACCESSOR_H
#define TABLE_INFO_ACCESSOR_H

// casa includes
#include <tables/Tables/Table.h>

// boost includes
#include <boost/shared_ptr.hpp>

// own includes
#include <dataaccess/ITableInfoAccessor.h>
#include <dataaccess/TableManager.h>

namespace askap {

namespace synthesis {

/// @brief Implementation of ITableInfoAccessor
/// @details This file contains a class, which just returns a reference
/// to SubtableInfoHolder stored in a smart pointer. This additional
/// level of wrapping is used to ship around the measurement set table
/// with its derived information.
/// @note see the TableManager class, which is an implementation of
/// the ISubtableInfoHolder and ITableHolder interfaces, for detailed
/// description how this bunch of classes is supposed to work together
/// @ingroup dataaccess_tm
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
  /// @param useMemBuffer if true, buffers in memory will be created
  /// instead of the disk-based buffers
  /// @param[in] dataColumn a name of the data column used by default
  TableInfoAccessor(const casa::Table &tab, bool useMemBuffer=false,
                    const std::string &dataColumn = "DATA"); 
  
  /// @return a non-const reference to Table held by this object
  virtual casa::Table& table() const throw();

  /// @return a reference to ISubtableInfoHolder
  virtual const ISubtableInfoHolder& subtableInfo() const;
  
  /// @return a reference to IMiscTableInfoHolder
  virtual const IMiscTableInfoHolder& miscTableInfo() const;

  /// @return a shared pointer on infoHolder
  virtual const boost::shared_ptr<ITableManager const>&
                        getTableManager() const throw();
  
private:
  boost::shared_ptr<ITableManager const> itsTableManager;
};

} // namespace synthesis

} // namespace askap

#endif // #define TABLE_INFO_ACCESSOR_H
