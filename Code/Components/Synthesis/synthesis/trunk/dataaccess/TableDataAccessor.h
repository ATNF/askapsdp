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
#include <dataaccess/ScratchBuffer.h>
#include <dataaccess/TableDataIterator.h>

namespace conrad {
	
namespace synthesis {

/// @brief an implementation of IDataAccessor
///
/// @details TableDataAccessor is an implementation of the
/// DataAccessor working with TableDataIterator.
class TableDataAccessor : public TableConstDataAccessor
{
public:
  /// construct an object linked with the given iterator
  /// @param iter a reference to associated iterator
  explicit TableDataAccessor(const TableConstDataIterator &iter);

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

  /// @brief set the scratch buffer to work with.
  /// @details The scratch buffer is a cache of the visibility cube
  /// and associated modification flags (one for reading, one for writing)
  /// @param[in] buf a shared pointer to scratch buffer
  void setBuffer(const boost::shared_ptr<ScratchBuffer> &buf) throw();

  /// revert to original visibilities
  void setOriginal() throw();

  /// set itsVisNeedsFlush to false (i.e. used after the visibility scratch 
  /// buffer is synchronized with the disk)
  void notifySyncCompleted() throw();

  /// @return True if the visibilities need to be written back
  bool visNeedsSync() const throw();

  /// @brief invalidate items updated on each iteration
  /// @details this method is overloaded to keep track of
  /// the iterator position and supply an appropriate piece
  /// of the active buffer. See TableConstDataAccessor for
  /// details on this methid.
  virtual void invalidateIterationCaches() const throw();

protected:
  /// read the information into the buffer if necessary
  void fillBufferIfNeeded() const;

  /// @brief Obtain a const reference to associated iterator.
  /// @details See documentation for the base TableConstDataAccessor class
  /// for more information
  /// @return a const reference to the associated iterator
  const TableDataIterator& iterator() const throw(DataAccessLogicError);

private:

  /// the current iteration of the active buffer. A void pointer means
  /// that the original visibility is selected, rather than a buffer.
  boost::shared_ptr<ScratchBuffer> itsScratchBufferPtr;

  /// a flag that the original visibility has been modified and needs
  /// flushing back
  bool itsVisNeedsFlush;
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef TABLE_DATA_ACCESSOR_H
