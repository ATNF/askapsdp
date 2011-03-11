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
#ifndef TABLE_BUFFER_DATA_ACCESSOR_H
#define TABLE_BUFFER_DATA_ACCESSOR_H

// boost includes
#include <boost/shared_ptr.hpp>

// own includes
#include <dataaccess/TableConstDataAccessor.h>
#include <dataaccess/IDataAccessor.h>
#include <dataaccess/ScratchBuffer.h>
#include <dataaccess/MetaDataAccessor.h>

namespace askap {
	
namespace accessors {

/// forward declaration of the type returned by reference
/// @ingroup dataaccess_tab
class TableDataIterator;

/// @brief an implementation of IDataAccessor
///
/// @details Thus is an implementation of the DataAccessor for writable
/// buffers working with TableDataIterator. This class is not
/// derived from TableConstDataAccessor, but instead is using it for most
/// of the operations. It appeared necessary because of the buffer(...)
/// methods of the iterator. To be able to return a persistent reference,
/// the iterator must maintain a collection of these accessors, one for
/// each buffer.
/// @ingroup dataaccess_tab
class TableBufferDataAccessor : virtual public MetaDataAccessor,
                                virtual public IDataAccessor
{
public:
  /// construct an object linked with the given const accessor and
  /// non-const iterator (which provides a read/write functionality)
  /// @param name a name of the buffer represented by this accessor
  /// @param iter a reference to the associated read-write iterator
  TableBufferDataAccessor(const std::string &name,
                                   const TableDataIterator &iter);

  /// Read-only visibilities (a cube is nRow x nChannel x nPol; 
  /// each element is a complex visibility)
  ///
  /// @return a reference to nRow x nChannel x nPol cube, containing
  /// all visibility data
  ///
  virtual const casa::Cube<casa::Complex>& visibility() const;
  
  
  /// Read-write access to visibilities (a cube is nRow x nChannel x nPol;
  /// each element is a complex visibility)
  ///
  /// @return a reference to nRow x nChannel x nPol cube, containing
  /// all visibility data
  ///
  virtual casa::Cube<casa::Complex>& rwVisibility();

  /// sync the buffer with table if necessary
  void sync();

  /// set needsRead flag to true (i.e. used following an iterator step
  /// to force updating the cache on the next data request
  void notifyNewIteration() throw();

private:
  /// read the information into the buffer if necessary
  void fillBufferIfNeeded() const;
    
  /// the current iteration of the buffer.
  mutable ScratchBuffer itsScratchBuffer;

  /// the name of the buffer (one needs it for a proper
  /// cache management request)
  const std::string itsName;

  /// @brief A reference to associated read-write iterator.
  /// @details
  /// @note We could have obtained
  /// it from the data accessor, but this approach seems more general and
  /// works faster.
  const TableDataIterator &itsIterator;
};


} // namespace accessors

} // namespace askap

#endif // #ifndef TABLE_BUFFER_DATA_ACCESSOR_H
