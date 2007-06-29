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

#include <algorithm>
#include <functional>
#include <utility>

#include <dataaccess/TableDataIterator.h>
#include <dataaccess/TableBufferDataAccessor.h>
#include <dataaccess/TableDataAccessor.h>
#include <dataaccess/TableInfoAccessor.h>
#include <dataaccess/IBufferManager.h>

namespace conrad {

namespace synthesis {

/// an adapter to use stl functions with maps.
/// can be moved in its own file, if found useful in other parts of the code
template<typename X>
struct MapMemFun : public std::unary_function<X*, void> {
  /// construct adapter for member function in
  /// @param[in] in member function
  explicit MapMemFun(void (X::*in)()) : func(in) {}

  /// @param[in] in a reference to the pair-like object with a pointer-like
  /// object stored in the second parameter.
  template<typename P>
  void operator()(const P &in) const    
    {
      CONRADDEBUGASSERT(in.second);
      return ((*in.second).*func)();
    }
private:
  void (X::*func)();
};

/// a helper method to autodetect the input type
/// @param[in] func member function to call for all elements in the map
template<typename X>
MapMemFun<X> mapMemFun(void (X::*in)()) {
  return MapMemFun<X>(in);
}

}
}

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
              TableConstDataIterator(msManager,sel,conv,maxChunkSize),
	      itsOriginalVisAccessor(new TableDataAccessor(getAccessor())),
	      itsIterationCounter(0)
{
  itsActiveBufferPtr=itsOriginalVisAccessor;
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
  std::map<std::string,
      boost::shared_ptr<TableBufferDataAccessor> >::const_iterator bufferIt =
                      itsBuffers.find(bufferID);
		      
  if (bufferIt==itsBuffers.end()) {
      // deal with new buffer
      itsActiveBufferPtr = itsBuffers[bufferID] =
            boost::shared_ptr<TableBufferDataAccessor>(
	          new TableBufferDataAccessor(bufferID,*this));
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
  std::map<std::string,
      boost::shared_ptr<TableBufferDataAccessor> >::const_iterator bufferIt =
                      itsBuffers.find(bufferID);
  if (bufferIt!=itsBuffers.end()) {
      // this buffer already exists
      return *(bufferIt->second);
  }
  // this is a request for a new buffer
  return *(itsBuffers[bufferID] =
      boost::shared_ptr<TableBufferDataAccessor>(new TableBufferDataAccessor(
                                       bufferID,*this)));
}

/// Restart the iteration from the beginning
void TableDataIterator::init()
{
  // call sync() member function for all accessors in itsBuffers
  std::for_each(itsBuffers.begin(),itsBuffers.end(),
           mapMemFun(&TableBufferDataAccessor::sync));

  TableConstDataIterator::init();
  itsIterationCounter=0;

  // call notifyNewIteration() member function for all accessors
  // in itsBuffers
  std::for_each(itsBuffers.begin(),itsBuffers.end(),
           mapMemFun(&TableBufferDataAccessor::notifyNewIteration));
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
  // call sync() member function for all accessors in itsBuffers
  std::for_each(itsBuffers.begin(),itsBuffers.end(),
           mapMemFun(&TableBufferDataAccessor::sync));

  ++itsIterationCounter;
  
  // call notifyNewIteration() member function for all accessors
  // in itsBuffers
  std::for_each(itsBuffers.begin(),itsBuffers.end(),
           mapMemFun(&TableBufferDataAccessor::notifyNewIteration));

  return TableConstDataIterator::next();
}

/// populate the cube with the data stored in the given buffer  
/// @param[in] vis a reference to the nRow x nChannel x nPol buffer
///            cube to fill with the complex visibility data
/// @param[in] name a name of the buffer to work with
void TableDataIterator::readBuffer(casa::Cube<casa::Complex> &vis,
                        const std::string &name) const
{
  const IBufferManager &bufManager=subtableInfo().getBufferManager();
  if (bufManager.bufferExists(name,itsIterationCounter)) {
      bufManager.readBuffer(vis,name,itsIterationCounter);
  } else {
      const TableConstDataAccessor &accessor=getAccessor();
      vis.resize(accessor.nRow(),accessor.nChannel(), accessor.nPol());
  }
}

/// write the cube back to the given buffer  
/// @param[in] vis a reference to the nRow x nChannel x nPol buffer
///            cube to fill with the complex visibility data
/// @param[in] name a name of the buffer to work with
void TableDataIterator::writeBuffer(const casa::Cube<casa::Complex> &vis,
                         const std::string &name) const
{
  subtableInfo().getBufferManager().writeBuffer(vis,name,itsIterationCounter);
}

/// destructor required to sync buffers on the last iteration
TableDataIterator::~TableDataIterator()
{
  // call sync() member function for all accessors in itsBuffers
  std::for_each(itsBuffers.begin(),itsBuffers.end(),
           mapMemFun(&TableBufferDataAccessor::sync));
}
