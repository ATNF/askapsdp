/// @file SharedIter.h
///
/// SharedIter: a helper template to be used with iterators created by
///        factories, which are expected to be handled via a boost's smart
///        pointer. It allows to avoid an ugly syntax like *(*it), etc.
///        
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef I_SHARED_ITER_H
#define I_SHARED_ITER_H

#include <string>
#include <boost/shared_ptr.hpp>

#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>

namespace conrad {

namespace synthesis {

// a template to handle iterators via a pointer stored in the shared_ptr,
// provides a basic iterator interface
template<typename T> class SharedIter
{
public:    
    /// constructors generally just call the same type of the constructor
    /// from boost::shared_ptr<T>

    /// an empty constructor has a special meaning for SharedIter - it is
    /// the end of iteration recognized by comparison operators
    SharedIter() : itsSharedPtr() {}; // never throws

    /// constructors from a raw pointer
    template<typename Y> explicit SharedIter(Y *p) : 
	           itsSharedPtr(p) {};    		   
    
    /// constructors from a shared pointer, call the same constructor
    /// of the shared pointer object, this class owns
    SharedIter(boost::shared_ptr<T> const &r) :
	     itsSharedPtr(r) {}; // never throws
    template<typename Y> SharedIter(boost::shared_ptr<Y> const &r) :
             itsSharedPtr(r) {}; // never throws
    
    /// constructors from SharedIter, need templates to allow automatic
    /// type cast
    SharedIter(SharedIter<T> const &r) :
             itsSharedPtr(r.itsSharedPtr) {}; // never throws
    template<typename Y> SharedIter(SharedIter<Y> const &r) :
             itsSharedPtr(static_cast<boost::shared_ptr<Y> const&>(r)) {}; // never throws

    /// assignment operators. Check for self-assignment is done in shared_ptr,
    /// no need to repeat it here.
    inline SharedIter<T>& operator=(SharedIter<T> const &r) throw()
    {
      itsSharedPtr=r.itsSharedPtr;
      return *this;
    }

    template<typename Y>inline SharedIter<T>& operator=(SharedIter<Y> const &r) throw()
    {
      itsSharedPtr=static_cast<boost::shared_ptr<Y> const&>(r);
      return *this;
    }
            
    
    /// access via operator* uses typedef value_type defined in each
    /// iterator which can be used in conjunction with this class
    inline typename T::value_type operator*() const {
       DebugAssert(*this,casa::AipsError);
       return *(*itsSharedPtr);
    }

    /// access via operator-> uses typedef pointer_type defined in each
    /// iterator which can be used in conjunction with this class
    inline typename T::pointer_type operator->() const {
       DebugAssert(*this,casa::AipsError);
       return (*itsSharedPtr).operator->();
    }

    /// shortcuts to various iterator methods

    /// in contrast to IConstDataIterator::init(), SharedIter<T>::init()
    /// returns a const reference to the object itself. It allows to
    /// call it from the parameter line of an STL algorithm    
    ///
    /// @return a const reference to itself
    ///
    /// The method is named init() as opposed to STL's begin() to
    /// highlight that it does an initialization of existing object.
    inline const SharedIter<T>& init() const
    {
      DebugAssert(itsSharedPtr,casa::AipsError);
      itsSharedPtr->init();
      return *this;
    }
    
    /// @return  True if the are more data to iterate
    inline bool hasMore() const throw()
    {      
      if (itsSharedPtr) {
          return itsSharedPtr->hasMore();
      }
      return false;
    }
    

    /// @return true if there are more data (so constructions like
    ///         while (it.next()) {} are possible)
    inline bool next() const
    {
      DebugAssert(itsSharedPtr,casa::AipsError);
      return itsSharedPtr->next();      
    }

    /// increment operator (only prefix operator is used. Postfix
    /// increment doesn't make sense with shared pointers as we can't
    /// copy the actual iterator)
    ///
    /// @return a reference to itself to allow ++++it syntax
    inline const SharedIter<T>& operator++() const
    {      
      next();
      return *this;
    }

    /// switch the iterator to one of the associated buffers
    /// (this call translates directly to the appropriate
    /// IDataIterator call). The method is valid for types
    /// derived from IDataIterator only.
    ///
    /// @param bufferID the name of buffer to choose
    ///
    inline void chooseBuffer(const std::string &bufferID) const
    {
      DebugAssert(itsSharedPtr,casa::AipsError);
      itsSharedPtr->chooseBuffer(bufferID);
    }

    /// restore the original link between the accessor returned
    /// by this iterator and the original visibilities
    /// (see IDataIterator::chooseOriginal() for more details)
    inline void chooseOriginal() const {
      DebugAssert(itsSharedPtr,casa::AipsError);
      itsSharedPtr->chooseOriginal(); 
    }

    /// Access to a given buffer bypassing operator* and 
    /// chooseBuffer/chooseOriginal mechanism.
    /// The call is translated to the appropriate IDataIterator call
    /// (which should be provided by the underlying iterator class)
    ///
    /// @param bufferID the name of the buffer to return
    /// @return a reference to the data accessor corresponding to the
    ///         buffer requested
    ///
    inline typename T::value_type buffer(const std::string &bufferID) const {
       DebugAssert(itsSharedPtr,casa::AipsError);
       return itsSharedPtr->buffer(bufferID);
    }
    

    /// comparison operators provide comparison with the end flag only
    /// at this stage (and may be forever). Comparison is allowed only
    /// between SharedIter templates of the same type (can be changed if
    /// there is a good reason to do so).
    ///
    /// @param[in] cmp a reference to another SharedIter<T> object
    /// @return True of the iterator held by a non-empty object reaches its
    ///         end of iteration, False otherwise
    ///
    inline bool operator==(const SharedIter<T> &cmp) const {
         if (cmp.itsSharedPtr) {
	     // this class should be null
	     if (itsSharedPtr)
	         throw casa::AipsError("A comparison of SharedIter has only been implemented "
	                  "for the case where one of the objects is empty.");	     
	     return !cmp.hasMore(); // == the empty object means that
	                           // it is at the end	     
	 } else if (itsSharedPtr) {
	     return !hasMore(); // == the empty object means that
	                        // it is at the end
	 } 
	 throw casa::AipsError("A comparison of SharedIter has only been implemented " 
	                 "for the case where one of the objects is not empty.");
	 return false; 
    }

    /// @param[in] cmp a reference to another SharedIter<T> object
    /// @return True of the iterator held by a non-empty object reaches its
    ///         end of iteration, False otherwise
    ///
    inline bool operator!=(const SharedIter<T> &cmp) const {
         if (cmp.itsSharedPtr) {
	     // this class should be null
	     if (itsSharedPtr)
	         throw casa::AipsError("A comparison of SharedIter has only been implemented "
	                  "for the case where one of the objects is empty.");	     
	     return cmp.hasMore(); // != the empty object means that
	                           // it is not at the end	     
	 } else if (itsSharedPtr) {
	     return hasMore(); // == the empty object means that
	                        // it is not at the end
	 } 
	 throw casa::AipsError("A comparison of SharedIter has only been implemented " 
	                 "for the case where one of the objects is not empty.");
	 return true;
    }

    /// return an empty SharedIter of the same type as the current object
    /// This method makes all calls to STL algorithms nicer
    /// @return An empty object (default constructor) of the same type
    ///         as this one
    inline SharedIter<T> end() const throw()
    {
       return SharedIter<T>();
    }

    /// call reset method of the shared pointer (i.e. force a release of
    /// this particular reference on the iterator, before the wrapping object
    /// goes out of scope). Effectively this call makes the iterator an
    /// end mark, until a new assignment has been made. The method is named
    /// 'release' in contrast to 'reset' for the shared pointer to avoid
    /// incorrect associations with rewinding of the iterator.
    inline void release() throw()
    {
       itsSharedPtr.reset();
    }

    /// type conversion operator to be able to get the shared pointer
    /// const and non-const versions
    
    /// @return a const reference to the wrapped shared pointer
    inline operator const boost::shared_ptr<T>&() const throw()
    {
       return itsSharedPtr;
    }
    /// @return a non-const reference to the wrapped shared pointer
    inline operator boost::shared_ptr<T>&() throw()
    {
       return itsSharedPtr;
    }
private:
   // actual shared pointer to the iterator
   boost::shared_ptr<T> itsSharedPtr;
};

class IDataIterator;
class IConstDataIterator;

typedef SharedIter<IDataIterator> IDataSharedIter;
typedef SharedIter<IConstDataIterator> IConstDataSharedIter;

} // end of namespace synthesis

} // end of namespace conrad

#endif // #ifndef I_SHARED_ITER_H
