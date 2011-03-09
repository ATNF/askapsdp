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

#include <dataaccess/TableHolder.h>

using namespace askap::synthesis;

/// constructor - set the table to work with
/// @param[in] tab table to work with
TableHolder::TableHolder(const casa::Table &tab) :
              itsTable(tab) {}


/// @return a non-const reference to Table held by this object
casa::Table& TableHolder::table() const throw()
{
  return itsTable;
}
