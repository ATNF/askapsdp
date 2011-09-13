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
  if (itsChangedFlag) {
      ASKAPCHECK(!itsFlushFlag, "An attempt to do read on-demand when the cache needs flush, this is most likely a logical error");     
	  (reader.*func)(itsValue);
	  itsChangedFlag=false;
  }
  return itsValue;
}


template<class T> template<typename Reader>
const T& CachedAccessorField<T>::value(Reader reader) const
{ 
  if (itsChangedFlag) {
      ASKAPCHECK(!itsFlushFlag, "An attempt to do read on-demand when the cache needs flush, this is most likely a logical error");     
	  reader(itsValue);
	  itsChangedFlag=false;
  }
  return itsValue;
}

template<class T>
const T& CachedAccessorField<T>::value() const
{
  ASKAPCHECK(!itsChangedFlag, "An attempt to use CachedAccessorField<T>::value() when read operation is required, most likely a logical error");
  return itsValue;
}

template<class T> template<typename Reader>
inline T& CachedAccessorField<T>::rwValue(const Reader &reader, void (Reader::*func)(T&) const) {
  // to ensure read on-demand
  value(reader,func);
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
  // flush will be needed
  itsFlushFlag = true; 
  // to get non-const reference
  return itsValue;
}

template<class T>
inline T& CachedAccessorField<T>::rwValue() const
{
  ASKAPCHECK(!itsChangedFlag, "An attempt to use CachedAccessorField<T>::rwValue() when read operation is required, most likely a logical error");
  // flush will be needed
  itsFlushFlag = true; 
  return itsValue;
}

} // namespace accessors

} // namespace askap

#endif // #ifndef CACHED_ACCESSOR_FIELD_TCC

