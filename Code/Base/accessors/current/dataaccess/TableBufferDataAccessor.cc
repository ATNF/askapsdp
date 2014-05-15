/// @file
/// @brief an implementation of IDataAccessor for buffers
///
/// @details TableDataAccessor is an implementation of the
/// DataAccessor working with TableDataIterator. It deals with
/// writable buffers only. Another class TableDataAccessor is
/// intended to write to the original visibility data.
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

/// own includes
#include <dataaccess/TableBufferDataAccessor.h>
#include <dataaccess/DataAccessError.h>
#include <dataaccess/TableDataIterator.h>

using namespace askap;
using namespace askap::accessors;

/// construct an object linked with the given const accessor and
/// non-const iterator (which provides a read/write functionality)
/// @param name a name of the buffer represented by this accessor
/// @param iter a reference to the associated read-write iterator
TableBufferDataAccessor::TableBufferDataAccessor(const std::string &name,
                                   const TableDataIterator &iter) :
                 MetaDataAccessor(iter.getAccessor()), itsName(name),
			     itsIterator(iter) {}

/// Read-only visibilities (a cube is nRow x nChannel x nPol; 
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
const casa::Cube<casa::Complex>& TableBufferDataAccessor::visibility() const
{
  // active buffer should be returned
  fillBufferIfNeeded();
  return itsScratchBuffer.vis;  
}

/// read the information into the buffer if necessary
void TableBufferDataAccessor::fillBufferIfNeeded() const
{
  if (itsScratchBuffer.needsRead) {
      ASKAPDEBUGASSERT(!itsScratchBuffer.needsFlush);
      itsIterator.readBuffer(itsScratchBuffer.vis, itsName);      
      itsScratchBuffer.needsRead=false;
  }
}

/// Read-write access to visibilities (a cube is nRow x nChannel x nPol;
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
casa::Cube<casa::Complex>& TableBufferDataAccessor::rwVisibility()
{    
  fillBufferIfNeeded();
  itsScratchBuffer.needsFlush=true;
  return itsScratchBuffer.vis;      
}

/// set needsRead flag to true (i.e. used following an iterator step
/// to force updating the cache on the next data request
void TableBufferDataAccessor::notifyNewIteration() throw()
{
  itsScratchBuffer.needsRead=true;
}

/// sync the buffer with table if necessary
void TableBufferDataAccessor::sync()
{
  if (itsScratchBuffer.needsFlush) {
     // sync with the table
     itsIterator.writeBuffer(itsScratchBuffer.vis, itsName);
     itsScratchBuffer.needsFlush=false;
  }
}
