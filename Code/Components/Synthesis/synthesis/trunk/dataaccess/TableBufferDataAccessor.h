/// @file
/// @brief an implementation of IDataAccessor for buffers
///
/// @details TableDataAccessor is an implementation of the
/// DataAccessor working with TableDataIterator. It deals with
/// writable buffers only. Another class TableDataAccessor is
/// intended to write to the original visibility data.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
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

namespace conrad {
	
namespace synthesis {

/// forward declaration of the type returned by reference
/// @ingroup dataaccess
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
/// @note conceptually, the class should be derived from MetaDataAccessor and
/// IDataAccessor and MetaDataAccessor should be derived from IConstDataAccessor
/// rather then from IDataAccessor. However, the correct approach requires
/// a virtual inheritance from all interfaces and it won't work with g++-3.3
/// @ingroup dataaccess
class TableBufferDataAccessor : virtual public MetaDataAccessor
{
public:
  /// construct an object linked with the given const accessor and
  /// non-const iterator (which provides a read/write functionality)
  /// @param name a name of the buffer represented by this accessor
  /// @param iter a reference to the associated read-write iterator
  explicit TableBufferDataAccessor(const std::string &name,
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
    
  /// the current iteration of the active buffer. A void pointer means
  /// that the original visibility is selected, rather than a buffer.
  mutable ScratchBuffer itsScratchBuffer;

  /// the name of the current buffer (one needs it for a proper
  /// cache management request)
  const std::string itsName;

  /// @brief A reference to associated read-write iterator.
  /// @details
  /// @note We could have obtained
  /// it from the data accessor, but this approach seems more general and
  /// works faster.
  const TableDataIterator &itsIterator;
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef TABLE_BUFFER_DATA_ACCESSOR_H
