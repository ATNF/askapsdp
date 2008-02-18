/// @file
/// @brief A stubbed iterator returning the given accessor
/// @details Imaging code is currently working with iterators rather than
/// individual accessors. Therefore, it is hard to integrate it with calibration
/// without multiple iterations over the dataset. Converting the code to use
/// accessors is necessary, but seems to be a lot of the job. This iterator is
/// a (temporary) adapter, which just returns supplied accessor as its value.
/// Only one chunk is defined.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include <dataaccess/FakeSingleStepIterator.h>
#include <dataaccess/MemBufferDataAccessor.h>
#include <conrad/ConradError.h>

using namespace conrad;
using namespace conrad::synthesis;

/// @brief initialize stubbed iterator
FakeSingleStepIterator::FakeSingleStepIterator() : itsOriginFlag(true) {}
	
	
/// @details 
/// operator* delivers a reference to data accessor (current chunk)
///
/// @return a reference to the current chunk
IDataAccessor& FakeSingleStepIterator::operator*() const
{ 
  CONRADCHECK(itsDataAccessor,
              "Data accessor has to be assigned first to FakeSingleStepIterator");
  CONRADDEBUGASSERT(itsOriginFlag);
  return *itsDataAccessor;
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
void FakeSingleStepIterator::chooseBuffer(const std::string &bufferID)
{
  CONRADTHROW(ConradError,"chooseBuffer is not implemented for FakeSingleStepIterator");
}

/// Switch the output of operator* and operator-> to the original
/// state (present after the iterator is just constructed) 
/// where they point to the primary visibility data. This method
/// is indended to cancel the results of chooseBuffer(casa::uInt)
///
void FakeSingleStepIterator::chooseOriginal()
{
  CONRADTHROW(ConradError,"chooseOriginal is not implemented for FakeSingleStepIterator");
}

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
IDataAccessor& FakeSingleStepIterator::buffer(const std::string &bufferID) const
{
  CONRADTHROW(ConradError,"buffer is not implemented for FakeSingleStepIterator, "
              "attempting to access "<<bufferID);
}

/// advance the iterator one step further
///
/// @return A reference to itself (to allow ++++it synthax)
///
/// The default implementation is via next(), however one can
/// override this method in a derived class to avoid this (slight)
/// overhead. This method overrides the method of the base
/// class to return the correct type 
IDataIterator& FakeSingleStepIterator::operator++()
{
  CONRADDEBUGASSERT(itsOriginFlag);
  itsOriginFlag = false;
  return *this;
}
	
/// Restart the iteration from the beginning
void FakeSingleStepIterator::init()
{
  itsOriginFlag = true;
}
	
/// Checks whether there are more data available.
/// @return True if there are more data available
casa::Bool FakeSingleStepIterator::hasMore() const throw()
{
  return itsOriginFlag;
}
	
/// advance the iterator one step further
/// @return True if there are more data (so constructions like
///         while(it.next()) {} are possible)
casa::Bool FakeSingleStepIterator::next()
{
  itsOriginFlag = false;
  return false; // we have just one element to iterate over
}

namespace conrad {

namespace utility {

/// @brief helper class - null deleter
/// @details To prevent boost::shared_ptr from disposing of an object
/// passed by reference. Can be moved to the upper level, if needed somewhere else
struct NullDeleter {
   /// @brief dummy method
   void operator()(void const *) const {}
};

} // namespace utility

} // namespace conrad

/// @brief assign a read/write accessor to this iterator
/// @details itsDataAccessor is initialized with a reference
/// to the given accessor. Note, reference semantics is used.
/// itsDataAccessor doesn't have the ownership of the pointer
/// and therefore doesn't delete it if this object is destroyed.
/// @param[in] acc a reference to data accessor (non-const)
void FakeSingleStepIterator::assignDataAccessor(IDataAccessor &acc)
{
  itsDataAccessor.reset(&acc,utility::NullDeleter());
}

/// @brief assign a const accessor to this iterator
/// @details itsDataAccessor is initialized with a new instance of
/// MemBufferDataAccessor initialized with the reference to the given
/// const data accessor. Note that the reference semantics is still
/// used, since MemBufferDataAccessor is invalid without a valid
/// const accessor it refers to. 
/// Data accessor passed as a parameter is not destroyed when this
/// class goes out of scope.
/// @param[in] acc a const reference to const data accessor
void FakeSingleStepIterator::assignConstDataAccessor(const IConstDataAccessor &acc)
{
  itsDataAccessor.reset(new MemBufferDataAccessor(acc));
}

/// @brief detach this iterator from current accessor
/// @details Because the reference semantics is used, it is not practical
/// to keep this iterator assigned to an accessor for longer than needed.
/// Otherwise, it is possible that the accessor becomes invalid first.
/// This method is intended to be called when all access operations to
/// the given accessor are completed. This makes the code safer, although
/// nothing bad would happen if this iterator is not accessed when 
/// associated accessor is not valid (i.e. there is no logical error in the
/// other places of the code).
void FakeSingleStepIterator::detachAccessor()
{
  itsDataAccessor.reset();
}
