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

// stl includes
#include <algorithm>
#include <functional>
#include <utility>

// own includes
#include <dataaccess/TableDataIterator.h>
#include <dataaccess/TableBufferDataAccessor.h>
#include <dataaccess/TableDataAccessor.h>
#include <dataaccess/TableInfoAccessor.h>
#include <dataaccess/IBufferManager.h>
#include <dataaccess/DataAccessError.h>

// casa includes
#include <tables/Tables/ArrayColumn.h>

namespace conrad {

namespace synthesis {

/// @brief an adapter to use stl functions with maps.
/// @details
/// can be moved in its own file, if found useful in other parts of the code
/// @ingroup dataaccess_hlp
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
  /// @brief buffered pointer to a member function
  void (X::*func)();
};

/// a helper method to autodetect the input type
/// @param[in] in a member function to call for all elements in the map
template<typename X>
MapMemFun<X> mapMemFun(void (X::*in)()) {
  return MapMemFun<X>(in);
}

}
}

using namespace conrad;
using namespace conrad::synthesis;

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
	      itsOriginalVisAccessor(new TableDataAccessor(*this)),
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
  CONRADDEBUGASSERT(itsOriginalVisAccessor);         
  itsOriginalVisAccessor->sync();

  TableConstDataIterator::init();
  itsIterationCounter=0;

  // call notifyNewIteration() member function for all accessors
  // in itsBuffers
  std::for_each(itsBuffers.begin(),itsBuffers.end(),
           mapMemFun(&TableBufferDataAccessor::notifyNewIteration));
  // original visibilities will be read on-demand by the code in
  // TableConstDataAccessor in a usual way
}

     
/// advance the iterator one step further 
/// @return True if there are more data (so constructions like 
///         while(it.next()) {} are possible)
casa::Bool TableDataIterator::next()
{
  // call sync() member function for all accessors in itsBuffers
  std::for_each(itsBuffers.begin(),itsBuffers.end(),
           mapMemFun(&TableBufferDataAccessor::sync));
  CONRADDEBUGASSERT(itsOriginalVisAccessor);         
  itsOriginalVisAccessor->sync();

  ++itsIterationCounter;
  
  // call notifyNewIteration() member function for all accessors
  // in itsBuffers
  std::for_each(itsBuffers.begin(),itsBuffers.end(),
           mapMemFun(&TableBufferDataAccessor::notifyNewIteration));
  // original visibilities will be read on-demand by the code in
  // TableConstDataAccessor in a usual way

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
  const TableConstDataAccessor &accessor=getAccessor();
  const casa::IPosition requiredShape(3, accessor.nRow(),
          accessor.nChannel(), accessor.nPol());
  if (bufManager.bufferExists(name,itsIterationCounter)) {
      bufManager.readBuffer(vis,name,itsIterationCounter);
      if (vis.shape()!=requiredShape) {
	  // this is an old buffer with a different shape. Can't be used
	  vis.resize(requiredShape);
      }
  } else {     
      vis.resize(requiredShape);
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
  if (itsOriginalVisAccessor) {
      // there is no much point to throw an exception here if itsOriginalVisAccesor
      // doesn't point to a valid instance for some reason (it shouldn't happend)         
      itsOriginalVisAccessor->sync();
  }    
}

/// @brief write back the original visibilities
/// @details The write operation is possible if the shape of the 
/// visibility cube stays the same as the shape of the data in the
/// table. The method uses DataAccessor to obtain a reference to the
/// visibility cube (hence no parameters). 
void TableDataIterator::writeOriginalVis() const
{
  const casa::Cube<casa::Complex> &originalVis=getAccessor().visibility();
  // no change of shape is permitted
  CONRADASSERT(originalVis.nrow() == nRow() &&
               originalVis.ncolumn() == nChannel() &&
               originalVis.nplane() == nPol());
  casa::ArrayColumn<casa::Complex> visCol(getCurrentIteration(), "DATA");
  CONRADDEBUGASSERT(getCurrentIteration().nrow() >= getCurrentTopRow()+
                    nRow());
  casa::uInt tableRow = getCurrentTopRow();
  for (casa::uInt row=0;row<originalVis.nrow();++row,++tableRow) {
       const casa::IPosition &shape = visCol.shape(row);
       CONRADDEBUGASSERT(shape.size() && (shape.size())<3);
       const casa::uInt thisRowNumberOfPols = shape[0];
       const casa::uInt thisRowNumberOfChannels = shape.size()>1 ? shape[1] : 1;
       if (thisRowNumberOfPols != originalVis.nplane() ||
           thisRowNumberOfChannels != originalVis.ncolumn()) {
           CONRADTHROW(DataAccessError, "Current implementation of the writing to original "
                "visibilities does not support partial selection of the data");                 
       }
       // for now just copy
       casa::IPosition curPos(2,thisRowNumberOfPols,
                                 thisRowNumberOfChannels);
       casa::Array<casa::Complex> buf(curPos);
       for (casa::uInt chan=0; chan<thisRowNumberOfChannels; ++chan) {
            curPos[1]=chan;
            for (casa::uInt pol=0; pol<thisRowNumberOfPols; ++pol) {
                 curPos[0] = pol;
                 buf(curPos) = originalVis(row,chan,pol);
            }
       }
       visCol.put(tableRow, buf);                          
  }             
}		  

/// @brief check whether one can write to the main table
/// @details Buffers held in subtables are not covered by this method.
/// @return true if write operation is allowed
bool TableDataIterator::mainTableWritable() const throw()
{
  try {
    return getCurrentIteration().isWritable();
  }
  catch (...) {}
  return false;
}
