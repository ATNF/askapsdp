/// @file
/// @brief a single cached field of the data accessor 
///
/// @details TableConstDataAccessor manages a number of cached fields.
/// This class represents a single such field
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include <askap/AskapError.h>

#ifndef CACHED_ACCESSOR_FIELD_TCC
#define CACHED_ACCESSOR_FIELD_TCC


namespace askap {

namespace accessors {

template<class T> template<typename Reader>
const T& CachedAccessorField<T>::value(const Reader &reader, 
                        void (Reader::*func)(T&) const)  const
{ 
#ifdef _OPENMP
  boost::upgrade_lock<boost::shared_mutex> lock(itsMutex);
  if (itsChangedFlag) {
      boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
#endif
      if (itsChangedFlag) {
          ASKAPCHECK(!itsFlushFlag, "An attempt to do read on-demand when the cache needs flush, this is most likely a logical error");     
	      (reader.*func)(itsValue);
	      itsChangedFlag=false;
	  }
#ifdef _OPENMP
  }
#endif  
  return itsValue;
}


template<class T> template<typename Reader>
const T& CachedAccessorField<T>::value(Reader reader) const
{ 
#ifdef _OPENMP
  boost::upgrade_lock<boost::shared_mutex> lock(itsMutex);
  if (itsChangedFlag) {
      boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
#endif
      if (itsChangedFlag) {
          ASKAPCHECK(!itsFlushFlag, "An attempt to do read on-demand when the cache needs flush, this is most likely a logical error");     
  	      reader(itsValue);
	      itsChangedFlag=false;
	  }
#ifdef _OPENMP
  }
#endif
  return itsValue;
}

template<class T>
const T& CachedAccessorField<T>::value() const
{
  ASKAPCHECK(!isChanged(), "An attempt to use CachedAccessorField<T>::value() when read operation is required, most likely a logical error");
  return itsValue;
}

template<class T> template<typename Reader>
inline T& CachedAccessorField<T>::rwValue(const Reader &reader, void (Reader::*func)(T&) const) {
  // to ensure read on-demand
  value(reader,func);
  
  // we don't synchronise writes because more action is required by the code which actually does write something
  // using the reference returned
    
  // flush will be needed
  itsFlushFlag = true; 
  // to get non-const reference
  return itsValue;
}

template<class T> template<typename Reader>
inline T& CachedAccessorField<T>::rwValue(Reader reader)
{
  // to ensure read on-demand
  value(reader);

  // we don't synchronise writes because more action is required by the code which actually does write something
  // using the reference returned

  // flush will be needed
  itsFlushFlag = true; 
  // to get non-const reference
  return itsValue;
}

template<class T>
inline T& CachedAccessorField<T>::rwValue() const
{
  ASKAPCHECK(!isChanged(), "An attempt to use CachedAccessorField<T>::rwValue() when read operation is required, most likely a logical error");
  // flush will be needed
  itsFlushFlag = true; 
  return itsValue;
}

/// @brief helper method to check if the cache needs an update
/// @details This method has been introduced to provide better encapsulation of
/// the synchronisation code if thread safety is required
/// @return true, if the cache needs update
template<class T>
inline bool CachedAccessorField<T>::isChanged() const
{
#ifdef _OPENMP
  boost::shared_lock<boost::shared_mutex> lock(itsMutex);
#endif
  return itsChangedFlag;
}

#ifdef _OPENMP
/// @brief copy constructor
/// @param[in] other an object to copy from
/// @note reference semantics for casa arrays, but we're not copying this class where T is a casa array type. 
template<class T>
CachedAccessorField<T>::CachedAccessorField(const CachedAccessorField<T> &other) : itsChangedFlag(true),
        itsFlushFlag(false) 
{
  boost::shared_lock<boost::shared_mutex> readLock(other.itsMutex);
  itsChangedFlag = other.itsChangedFlag;
  itsFlushFlag = other.itsFlushFlag;
  itsValue = other.itsValue;
}
#endif

/// @brief assignment operator
/// @param[in] other an object to copy from
/// @note reference semantics for casa arrays, but we're not using this method where T is a casa array type. 
template<class T>
CachedAccessorField<T>& CachedAccessorField<T>::operator=(const CachedAccessorField<T> &other)
{
  if (&other != this) {
      
#ifdef _OPENMP
      boost::shared_lock<boost::shared_mutex> readLock(other.itsMutex);
      boost::unique_lock<boost::shared_mutex> lock(itsMutex);
#endif
      
      itsChangedFlag = other.itsChangedFlag;
      itsFlushFlag = other.itsFlushFlag;
      itsValue = other.itsValue;
      // deliberately don't copy mutex as it is object-specific     
  }
  return *this;
}


} // namespace accessors

} // namespace askap

#endif // #ifndef CACHED_ACCESSOR_FIELD_TCC

