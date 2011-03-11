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

#ifndef I_TABLE_HOLDER_H
#define I_TABLE_HOLDER_H

// casa includes
#include <tables/Tables/Table.h>

// own includes
#include <dataaccess/IHolder.h>

namespace askap {

namespace accessors {

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


} // namespace accessors

} // namespace askap

#endif // #ifndef I_TABLE_HOLDER_H
