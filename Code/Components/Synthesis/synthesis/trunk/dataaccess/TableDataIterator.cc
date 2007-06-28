/// @file
///
/// @brief Implementation of IDataIterator in the table-based case
/// @details
/// TableConstDataIterator: Allow read-only iteration across preselected data. Each 
/// iteration step is represented by the IConstDataAccessor interface.
/// TableDataIterator extends the interface further to read-write operations.
/// Each iteration step is represented by the IDataAccessor interface in this
/// case. 
/// 
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include <dataaccess/TableDataIterator.h>
#include <dataaccess/TableBufferDataAccessor.h>
#include <dataaccess/TableInfoAccessor.h>

using namespace conrad;
using namespace synthesis;

/// @param[in] msManager a manager of the measurement set to use
/// @param[in] sel shared pointer to selector
/// @param[in] conv shared pointer to converter
/// @param[in] maxChunkSize maximum number of rows per accessor
TableDataIterator::TableDataIterator(
            const boost::shared_ptr<ITableManager const> &msManager,
            const boost::shared_ptr<ITableDataSelectorImpl const> &sel,
            const boost::shared_ptr<IDataConverterImpl const> &conv,
            casa::uInt maxChunkSize) : TableInfoAccessor(msManager),
              TableConstDataIterator(msManager,sel,conv,maxChunkSize)
{  
}

/// @brief operator* delivers a reference to data accessor (current chunk)
/// @details
/// @return a reference to the current chunk
/// @note
/// constness of the return type is changed to allow read/write
/// operations.
///
IDataAccessor& TableDataIterator::operator*() const
{
  CONRADDEBUGASSERT(itsActiveBufferPtr);
  return *itsActiveBufferPtr;
}

/// @brief Switch the output of operator* and operator-> to one of 
/// the buffers.
/// @details This is meant to be done to provide the same 
/// interface for a buffer access as exists for the original 
/// visibilities (e.g. it->visibility() to get the cube).
/// It can be used for an easy substitution of the original 
/// visibilities to ones stored in a buffer, when the iterator is
/// passed as a parameter to mathematical algorithms.   
/// The operator* and operator-> will refer to the chosen buffer
/// until a new buffer is selected or the chooseOriginal() method
/// is executed to revert operators to their default meaning
/// (to refer to the primary visibility data).  
/// @param[in] bufferID  the name of the buffer to choose
void TableDataIterator::chooseBuffer(const std::string &bufferID)
{
  std::map<std::string, boost::shared_ptr<IDataAccessor> >::const_iterator
                      bufferIt = itsBuffers.find(bufferID);
  if (bufferIt==itsBuffers.end()) {
      // deal with new buffer
      itsBuffers[bufferID] = itsActiveBufferPtr =
           boost::shared_ptr<IDataAccessor>(new TableBufferDataAccessor(getAccessor()));
  } else {
      itsActiveBufferPtr=bufferIt->second;
  }
}

/// Switch the output of operator* and operator-> to the original
/// state (present after the iterator is just constructed) 
/// where they point to the primary visibility data. This method
/// is indended to cancel the results of chooseBuffer(std::string)
///
void TableDataIterator::chooseOriginal()
{
  itsActiveBufferPtr=itsOriginalVisAccessor;
}

/// @brief obtain any associated buffer for read/write access.
/// @details The buffer is identified by its bufferID. The method 
/// ignores a chooseBuffer/chooseOriginal setting.  
/// @param[in] bufferID the name of the buffer requested
/// @return a reference to writable data accessor to the
///         buffer requested
IDataAccessor& TableDataIterator::buffer(const std::string &bufferID) const
{
  std::map<std::string, boost::shared_ptr<IDataAccessor> >::const_iterator
                      bufferIt = itsBuffers.find(bufferID);
  if (bufferIt!=itsBuffers.end()) {
      // this buffer already exists
      return *(bufferIt->second);
  }
  // this is a request for a new buffer
  return *(itsBuffers[bufferID] =
      boost::shared_ptr<IDataAccessor>(new TableBufferDataAccessor(getAccessor())));
}

/// Restart the iteration from the beginning
void TableDataIterator::init()
{
  TableConstDataIterator::init();
}

// hasMore method has to be overridden
// because of the problem with the g++-3.3 compiler (the inheritance
// between read/write and read/only interfaces can not be made virtual
// and therefore there is an ambiguity). Other methods are overridden anyway.

/// @brief Checks whether there are more data available.
/// @details
/// @return True if there are more data available
/// @note this method can be removed from this class
/// when we migrate to g++-4.1 and inheritance of
/// interfaces is made virtual.
casa::Bool TableDataIterator::hasMore() const throw()
{
  return TableConstDataIterator::hasMore();
}
      
/// advance the iterator one step further 
/// @return True if there are more data (so constructions like 
///         while(it.next()) {} are possible)
casa::Bool TableDataIterator::next()
{
  return TableConstDataIterator::next();
}
