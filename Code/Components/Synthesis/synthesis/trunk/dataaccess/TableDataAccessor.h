/// @file
/// @brief an implementation of IConstDataAccessor
///
/// @details TableDataAccessor is an implementation of the
/// DataAccessor working with TableDataIterator. 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef TABLE_DATA_ACCESSOR_H
#define TABLE_DATA_ACCESSOR_H

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
class TableDataIterator;

/// @brief an implementation of IDataAccessor
///
/// @details TableDataAccessor is an implementation of the
/// DataAccessor working with TableDataIterator. This class is not
/// derived from TableConstDataAccessor, but instead is using it for most
/// of the operations. It appeared necessary because of the buffer(...)
/// methods of the iterator. To be able to return a persistent reference,
/// the iterator must maintain a collection of these accessors, one for
/// each buffer.
/// @note conceptually, the class should be derived from MetaDataAccessor and
/// IDataAccessor and MetaDataAccessor should be derived from IConstDataAccessor
/// rather then from IDataAccessor. However, the correct approach requires
/// a virtual inheritance from all interfaces and it won't work with g++-3.3
class TableDataAccessor : virtual public MetaDataAccessor
{
public:
  /// construct an object linked with the given const accessor
  /// @param acc a reference to the associated accessor
  explicit TableDataAccessor(const TableConstDataAccessor &acc);

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

  /// set needsFlush fkag to false (i.e. used after the visibility scratch 
  /// buffer is synchronized with the disk)
  void notifySyncCompleted() throw();

  /// @return True if the visibilities need to be written back
  bool needSync() const throw();

  /// @brief invalidate items updated on each iteration
  /// @details this method is overloaded to keep track of
  /// the iterator position and supply an appropriate piece
  /// of the active buffer. See TableConstDataAccessor for
  /// details on this methid.
  virtual void invalidateIterationCaches() const throw();

protected:
  /// read the information into the buffer if necessary
  void fillBufferIfNeeded() const;

private:
    
  /// the current iteration of the active buffer. A void pointer means
  /// that the original visibility is selected, rather than a buffer.
  ScratchBuffer itsScratchBuffer;  
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef TABLE_DATA_ACCESSOR_H
