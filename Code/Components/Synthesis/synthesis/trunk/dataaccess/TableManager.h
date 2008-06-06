/// @file
/// @brief Class to manage Table and derived information
/// @details This file contains a class, which holds a table and
/// associated derived information, which is built on-demand. It
/// implements ITableHolder, IMiscTableInfoHolder and ISubtableInfoHolder interfaces.
/// At this stage, the class just has a constructor defined which
/// connects individual components together. All functionality is
/// inherited from the building blocks.
/// @note There are two ways of using this class:
/// @li At the level of hierarchy where the access to such information is
///    first required, derive also (multiple inheritance is assumed) virtually
///    from one of the interfaces like ITableHolder or ISubtableInfoHolder 
///    (depending on what is required,
///    the ITableManager interface is derived from all of them and provides both
///    the table and derived information access).
///    At the top level, derive also (multiple derivation is assumed) from
///    TableManager. This will implement pure virtual methods in interfaces.
///    This option is probably the best if only one class would need an 
///    access to the table and its associated info. 
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

#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H

// casa includes
#include <tables/Tables/Table.h>

// own includes
#include <dataaccess/ITableManager.h>
#include <dataaccess/SubtableInfoHolder.h>
#include <dataaccess/TableHolder.h>
#include <dataaccess/MiscTableInfoHolder.h>

// std includes
#include <string>

namespace askap {

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
///    from one of the interfaces like ITableHolder or ISubtableInfoHolder 
///    (depending on what is required,
///    the ITableManager interface is derived from all of them and provides both
///    the table and derived information access).
///    At the top level, derive also (multiple derivation is assumed) from
///    TableManager. This will implement pure virtual methods in interfaces.
///    This option is probably the best if only one class would need an 
///    access to the table and its associated info. 
/// @li Hold a pointer to this class in a smart pointer template, which can
///    be passed around for all interested classes. To provide a good
///    interface and avoid multiple copies of the code managing the
///    smart pointer, ITableInfoAccessor and TableInfoAccessor classes are
///    written. Similarly to the previous method, at the level of hierarchy
///    where an access to table/derived information is required, derive
///    virtually from ITableInfoAccessor. At the top level, derive
///    from TableInfoAccessor. Its constructor accepts a smart pointer
///    to ITableManager, which TableManager is derived from.
/// @ingroup dataaccess_tm
struct TableManager : virtual public ITableManager,
                      virtual public TableHolder,
                      virtual public SubtableInfoHolder,
                      virtual public MiscTableInfoHolder
{
  /// construct a table/derived info manager from the table object
  /// @param[in] tab MS table to work with
  /// @param[in] useMemBuffers if true, buffers in memory will be created
  /// instead of the disk-based buffers
  /// @param[in] dataColumn name of the data column used by default
  explicit TableManager(const casa::Table &tab, bool useMemBuffers,
                        const std::string &dataColumn = "DATA") :
           TableHolder(tab), SubtableInfoHolder(useMemBuffers),
           MiscTableInfoHolder(dataColumn) {}
};

} // namespace synthesis

} // namespace askap

#endif // #define TABLE_MANAGER_H
