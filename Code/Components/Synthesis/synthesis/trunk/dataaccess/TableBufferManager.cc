/// @file
/// @brief A class to manage buffers stored in subtable
/// @details Read-write iterator (see IDataIterator) uses the concept
/// of buffers to store scratch data. This class stores buffers in the
/// BUFFERS subtable 
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

// CASA includes
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ArrColDesc.h>
#include <casa/BasicSL/Complex.h>

// own includes
#include <dataaccess/TableBufferManager.h>
#include <dataaccess/DataAccessError.h>

#include <iostream>
using namespace std;

using namespace askap;
using namespace askap::synthesis;

/// construct the object and link it to the given buffers subtable
/// @param[in] tab  subtable to use
TableBufferManager::TableBufferManager(const casa::Table &tab) :
                   TableHolder(tab) {}


/// @brief populate the cube with the data stored in the given buffer
/// @details The method throws an exception if the requested buffer
/// does not exist (prevents a shape mismatch)
/// @param[in] vis a reference to the nRow x nChannel x nPol buffer
///            cube to fill with the complex visibility data
/// @param[in] name a name of the buffer to work with
/// @param[in] index a sequential index in the buffer
void TableBufferManager::readBuffer(casa::Cube<casa::Complex> &vis,
                        const std::string &name,
			   casa::uInt index) const
{ 
 ASKAPDEBUGASSERT(table().actualTableDesc().isColumn(name));
 ASKAPDEBUGASSERT(index<table().nrow());
 casa::ROArrayColumn<casa::Complex> bufCol(table(),name);
 ASKAPASSERT(bufCol.ndim(index) == 3); // only cubes should be in buffers
 bufCol.get(index,vis,casa::True);
}

/// @brief write the cube back to the given buffer
/// @details This buffer is created on the first write operation
/// @param[in] vis a reference to the nRow x nChannel x nPol buffer
///            cube to fill with the complex visibility data
/// @param[in] name a name of the buffer to work with
/// @param[in] index a sequential index in the buffer
void TableBufferManager::writeBuffer(const casa::Cube<casa::Complex> &vis,
                         const std::string &name,
			   casa::uInt index) const
{  
  if (!table().actualTableDesc().isColumn(name)) {
      // create a brand new buffer
      casa::ArrayColumnDesc<casa::Complex> newColDesc(name,
           "Writable buffer managed by the dataaccess layer",3);
      newColDesc.rwKeywordSet().define("UNIT","Jy");
      table().addColumn(newColDesc);
  }
  if (table().nrow()<=index) {
      table().addRow(index-table().nrow()+1);
  }
  casa::ArrayColumn<casa::Complex> bufCol(table(),name);
  bufCol.put(index,vis);
}


/// @brief check whether the particular buffer exists
/// @param[in] name a name of the buffer to query
/// @param[in] index a sequential index in the buffer
/// @return true, if the buffer with the given name is present
bool TableBufferManager::bufferExists(const std::string &name,
			   casa::uInt index) const
{
  if (!table().actualTableDesc().isColumn(name)) {
      // there is no such buffer at all
      return false;
  }
  if (table().nrow()<=index) {
      // buffer exists, but the index requested is beyond the limits
      return false;
  }
  casa::ROArrayColumn<casa::Complex> bufCol(table(),name);
  return bufCol.isDefined(index);
}
