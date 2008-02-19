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

#ifndef FAKE_SINGLE_STEP_ITERATOR_H
#define FAKE_SINGLE_STEP_ITERATOR_H

#include <string>
#include <map>

#include <boost/shared_ptr.hpp>

#include <dataaccess/IDataIterator.h>
#include <dataaccess/IDataAccessor.h>

namespace conrad {

namespace synthesis {

/// @brief An iterator stub to work in pair with DataAccessorStub
/// @ingroup dataaccess_hlp
class FakeSingleStepIterator : virtual public IDataIterator
{
public:
    /// @brief initialize stubbed iterator
	FakeSingleStepIterator();
		
	/// @details 
	/// operator* delivers a reference to data accessor (current chunk)
	///
	/// @return a reference to the current chunk
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
	/// overhead. This method overrides the method of the base
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
	
	/// @brief assign a read/write accessor to this iterator
	/// @details itsDataAccessor is initialized with a reference
	/// to the given accessor. Note, reference semantics is used.
	/// itsDataAccessor doesn't have the ownership of the pointer
	/// and therefore doesn't delete it if this object is destroyed.
	/// @param[in] acc a reference to data accessor (non-const)
	void assignDataAccessor(IDataAccessor &acc);

	/// @brief assign a const accessor to this iterator
	/// @details itsDataAccessor is initialized with a new instance of
	/// MemBufferDataAccessor initialized with the reference to the given
	/// const data accessor. Note that the reference semantics is still
	/// used, since MemBufferDataAccessor is invalid without a valid
	/// const accessor it refers to. 
	/// Data accessor passed as a parameter is not destroyed when this
	/// class goes out of scope.
	/// @param[in] acc a const reference to const data accessor
	void assignConstDataAccessor(const IConstDataAccessor &acc);
	
	/// @brief detach this iterator from current accessor
	/// @details Because the reference semantics is used, it is not practical
	/// to keep this iterator assigned to an accessor for longer than needed.
	/// Otherwise, it is possible that the accessor becomes invalid first.
	/// This method is intended to be called when all access operations to
	/// the given accessor are completed. This makes the code safer, although
	/// nothing bad would happen if this iterator is not accessed when 
	/// associated accessor is not valid (i.e. there is no logical error in the
	/// other places of the code).
	void detachAccessor();
protected:
    /// @brief helper method to reassign all buffers to a new accessor
    /// @details With the calls to assign/detach accessor it can be replaced
    ///	with a new reference. This method iterates over all buffers and reassigns
    /// them to the new accessor corresponding to the original visibilities.
    void reassignBuffers();
private:
    /// @brief flag whether this iterator is still at origin
    bool itsOriginFlag;
    
    /// @brief the name of the active buffer
    /// @details This data member is required because the iterator may have
    /// one of the buffers active, while it is made associated with a new 
    /// accessor. An empty string means that the original visibilities are 
    /// active (i.e. buffers are shadowed). It is a limitation of this class
    /// that empty buffer names are not allowed.
    std::string itsActiveBufferName;
	
	/// assigned data accessor
	boost::shared_ptr<IDataAccessor> itsDataAccessor;
	
	/// shared pointer to the data accessor associated with either an
	/// active buffer or the original accessor (default). The actual
	/// type held by pointer may vary.
	boost::shared_ptr<IDataAccessor> itsActiveAccessor;
	
	/// a container of buffers
	mutable std::map<std::string, 
	       boost::shared_ptr<IDataAccessor> > itsBuffers;
};

} // end of namespace synthesis

} // end of namespace conrad
#endif // #ifndef FAKE_SINGLE_STEP_ITERATOR_H
