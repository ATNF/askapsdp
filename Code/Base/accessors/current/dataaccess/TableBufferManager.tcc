/// @file
/// @brief A class to manage buffers stored in subtable
/// @details Read-write iterator (see IDataIterator) uses the concept
/// of buffers to store scratch data. This class stores buffers in the
/// BUFFERS subtable 
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

// CASA includes
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ArrColDesc.h>

// own includes
#include <dataaccess/DataAccessError.h>

#ifndef TABLE_BUFFER_MANAGER_TCC
#define TABLE_BUFFER_MANAGER_TCC

namespace askap {

namespace accessors {

/// @brief populate the cube with the data stored in the given table cell
/// @details The method throws an exception if the requested table cell
/// does not exist
/// @param[in] cube a reference to a cube of some type
/// @param[in] name a name of the column to work with
/// @param[in] index row number
template<typename T>
void TableBufferManager::readCube(casa::Cube<T> &cube, const std::string &name,
			    casa::uInt index) const
{ 
 ASKAPDEBUGASSERT(table().actualTableDesc().isColumn(name));
 ASKAPDEBUGASSERT(index<table().nrow());
 typename casa::ROArrayColumn<T> bufCol(table(),name);
 ASKAPASSERT(bufCol.ndim(index) == 3); // only cubes should be in buffers
 bufCol.get(index,cube,casa::True);
}

/// @brief write the cube back to the table
/// @details The table cell is populated with values on the first write 
/// operation
/// @param[in] cube to take the data from 
/// @param[in] name a name of the column to work with
/// @param[in] index row number
template<typename T>
void TableBufferManager::writeCube(const casa::Cube<T> &cube, const std::string &name,
			     casa::uInt index) const
{  
  if (!table().actualTableDesc().isColumn(name)) {
      // create a brand new buffer
      typename casa::ArrayColumnDesc<T> newColDesc(name,
           "Writable buffer managed by the dataaccess layer",3);
      newColDesc.rwKeywordSet().define("UNIT","Jy");
      table().addColumn(newColDesc);
  }
  if (table().nrow()<=index) {
      table().addRow(index-table().nrow()+1);
  }
  typename casa::ArrayColumn<T> bufCol(table(),name);
  bufCol.put(index,cube);
}

/// @brief check whether a particular table cell exists
/// @param[in] name a name of the table column to query
/// @param[in] index row number 
/// @return true, if the given cell exists and has an array
/// @note template type defined the type of the data
template<typename T>
bool TableBufferManager::cellDefined(const std::string &name,
			      casa::uInt index) const
{
  if (!table().actualTableDesc().isColumn(name)) {
      // there is no such column at all
      return false;
  }
  if (table().nrow()<=index) {
      // column exists, but the index requested is beyond the limits
      return false;
  }
  typename casa::ROArrayColumn<T> bufCol(table(),name);
  return bufCol.isDefined(index);
}

} // namespace accessors

} // namespace askap


#endif // #ifndef TABLE_BUFFER_MANAGER_TCC

