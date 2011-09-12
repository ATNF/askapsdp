/// @file
/// @brief a single cached field of the data accessor 
///
/// @details TableConstDataAccessor manages a number of cached fields.
/// This class represents a single such field
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef CACHED_ACCESSOR_FIELD_TCC
#define CACHED_ACCESSOR_FIELD_TCC

namespace askap {

namespace accessors {

template<class T> template<typename Reader>
const T& CachedAccessorField<T>::value(const Reader &reader, 
                        void (Reader::*func)(T&) const)  const
{ 
  if (itsChangedFlag) {
	  (reader.*func)(itsValue);
	  itsChangedFlag=false;
  }
  return itsValue;
}


template<class T> template<typename Reader>
const T& CachedAccessorField<T>::value(Reader reader) const
{ 
  if (itsChangedFlag) {
	  reader(itsValue);
	  itsChangedFlag=false;
  }
  return itsValue;
}

} // namespace accessors

} // namespace askap

#endif // #ifndef CACHED_ACCESSOR_FIELD_TCC

