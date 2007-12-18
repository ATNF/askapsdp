/// @file DataIteratorStub.h
/// @brief A stub to work in pair with DataAccessorStub
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef DATA_ITERATOR_STUB_H
#define DATA_ITERATOR_STUB_H

#include <string>

#include <dataaccess/IDataIterator.h>
#include <dataaccess/DataAccessorStub.h>

namespace conrad {

namespace synthesis {

/// @brief An iterator stub to work in pair with DataAccessorStub
/// @ingroup dataaccess_hlp
class DataIteratorStub : virtual public IDataIterator
{
public:
        /// initialize stubbed iterator,
	///
	/// @param nsteps  number of steps before the iterator
	///                reach the end
	///
        explicit DataIteratorStub(casa::uInt nsteps);
	
	/// Return the data accessor (current chunk) in various ways
	
	/// operator* delivers a reference to data accessor (current chunk)
	///
	/// @return a reference to the current chunk
	///
	/// constness of the return type is changed to allow read/write
	/// operations.
	///
	virtual IDataAccessor& operator*() const;
		
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
	virtual void chooseBuffer(const std::string &bufferID);

	/// Switch the output of operator* and operator-> to the original
	/// state (present after the iterator is just constructed) 
	/// where they point to the primary visibility data. This method
	/// is indended to cancel the results of chooseBuffer(casa::uInt)
	///
	virtual void chooseOriginal();

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
	virtual IDataAccessor& buffer(const std::string &bufferID) const;

	/// advance the iterator one step further
	///
	/// @return A reference to itself (to allow ++++it synthax)
	///
	/// The default implementation is via next(), however one can
	/// override this method in a derived class to avoid this (slight)
	/// overhead. This method overrides the the method of the base
	/// class to return the correct type 
	virtual IDataIterator& operator++();
	
	/// Restart the iteration from the beginning
	virtual void init();
	
	/// Checks whether there are more data available.
	/// @return True if there are more data available
	casa::Bool hasMore() const throw(); 
	
	/// advance the iterator one step further
	/// @return True if there are more data (so constructions like
	///         while(it.next()) {} are possible)
	casa::Bool next();
public:
        /// a maximum number of simulated iterations
	casa::uInt itsMaxCounter;
        /// counter of the iteration number
	casa::uInt itsCounter;
	/// accessor stub
	mutable DataAccessorStub itsAccessor;
};

} // end of namespace synthesis

} // end of namespace conrad
#endif // #ifndef I_DATA_ITERATOR_H
