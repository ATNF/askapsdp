/// @file
///
///
/// DataIteratorStub: A stub to work in pair with DataAccessorStub
///
/// 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include "DataIteratorStub.h"

namespace conrad {

namespace synthesis {

/// initialize stubbed iterator,
///
/// @param nsteps  number of steps before the iterator
///                reach the end
///
DataIteratorStub::DataIteratorStub(casa::uInt nsteps) : itsMaxCounter(nsteps),
             itsCounter(0), itsAccessor(true) {}

/// Return the data accessor (current chunk) in various ways

/// operator* delivers a reference to data accessor (current chunk)
///
/// @return a reference to the current chunk
///
/// constness of the return type is changed to allow read/write
/// operations.
///
IDataAccessor& DataIteratorStub::operator*() const {
   return itsAccessor;
}
	
/// Switch the output of operator* and operator-> to one of 
/// the buffers. This is meant to be done to provide the same 
/// interface for a buffer access as exists for the original 
/// visibilities (e.g. it->visibility() to get the cube).
/// It can be used for an easy substitution of the original 
/// visibilities to ones stored in a buffer, when the iterator is
/// passed as a parameter to mathematical algorithms. 
/// 
/// The operator* and operator-> will refer to the chosen buffer
/// until a new buffer is selected or the chooseOriginal() method
/// is executed to revert operators to their default meaning
/// (to refer to the primary visibility data).
///
/// @param[in] bufferID  the name of the buffer to choose
///
void DataIteratorStub::chooseBuffer(const std::string &)
{
}

/// Switch the output of operator* and operator-> to the original
/// state (present after the iterator is just constructed) 
/// where they point to the primary visibility data. This method
/// is indended to cancel the results of chooseBuffer(casa::uInt)
///
void DataIteratorStub::chooseOriginal() {}

/// return any associated buffer for read/write access. The 
/// buffer is identified by its bufferID. The method 
/// ignores a chooseBuffer/chooseOriginal setting.
/// 
/// @param[in] bufferID the name of the buffer requested
/// @return a reference to writable data accessor to the
///         buffer requested
///
/// Because IDataAccessor has both const and non-const visibility()
/// methods defined separately, it is possible to detect when a
/// write operation took place and implement a delayed writing
IDataAccessor& DataIteratorStub::buffer(const std::string &) const
{
  return itsAccessor;
}

/// advance the iterator one step further
///
/// @return A reference to itself (to allow ++++it synthax)
///
/// The default implementation is via next(), however one can
/// override this method in a derived class to avoid this (slight)
/// overhead. This method overrides the the method of the base
/// class to return the correct type 
IDataIterator& DataIteratorStub::operator++()
{
  ++itsCounter;
  return *this;  
}

/// Restart the iteration from the beginning
void DataIteratorStub::init()
{
   itsCounter=0;
}

/// Checks whether there are more data available.
/// @return True if there are more data available
casa::Bool DataIteratorStub::hasMore() const throw() {
  return itsCounter<itsMaxCounter;
}

/// advance the iterator one step further 
/// @return True if there are more data (so constructions like 
///         while(it.next()) {} are possible)
casa::Bool DataIteratorStub::next()
{  
  return ++itsCounter<itsMaxCounter;
}


} // end of namespace synthesis

} // end of namespace conrad
