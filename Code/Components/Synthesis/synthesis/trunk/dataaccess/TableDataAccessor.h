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

namespace conrad {
	
namespace synthesis {

/// @brief an implementation of IConstDataAccessor
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

  /// @brief set the buffer to work with.
  /// @details The modification flag (describing whether one needs to
  /// flush the buffer back to disk) is reset automatically.
  /// @param[in] buf shared pointer to nRow x nChannel x nPol cube with
  /// visibilities
  void setBuffer(const boost::shared_ptr<casa::Cube<casa::Complex> >
                 &buf) throw();

  /// revert to original visibilities
  void setOriginal() throw();

  /// set itsBufferNeedsFlush to false (i.e. after this buffer is
  /// synchronized with the disk
  void resetBufferFlushFlag() throw();

  /// @return True if the active buffer needs to be written back
  bool bufferNeedsFlush() const throw();
protected:
  /// read the information into the buffer if necessary
  void fillBufferIfNeeded() const;
private:

  /// the current iteration of the active buffer. A void pointer means
  /// that the original visibility is selected, rather than a buffer.
  boost::shared_ptr<casa::Cube<casa::Complex> > itsBufferedVisibility;

  /// a flag that the buffer was modified and needs flushing back
  bool itsBufferNeedsFlush;

  /// a flag meaning that the buffer has to be read from the disk
  /// (the same meaning as itsVisibilityChanged in the TableConstDataAccessor,
  /// but for buffers)
  mutable bool itsBufferChanged;
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef TABLE_DATA_ACCESSOR_H
