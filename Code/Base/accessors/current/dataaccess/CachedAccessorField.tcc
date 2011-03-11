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

/// @brief a single cached field of the data accessor 
///
/// @details TableConstDataAccessor manages a number of cached fields.
/// This class represents a single such field
/// Template parameter:
/// @li T is a type of the field
/// @ingroup dataaccess_hlp
template<typename T>
struct CachedAccessorField {
  /// @brief initialize the class, set the flag that reading is required
  CachedAccessorField() : itsChangedFlag(true) {}
  
  /// @brief access the data, read on-demand
  /// @details On the first request and whenever is necessary, this method reads the data using 
  /// a given member of the Reader class.
  /// Reader is a type of an object function, which can fill this field
  /// with the appropriate information (i.e. read it). 
  /// @param[in] reader an object which has a method able to fill this field
  /// @param[in] func a pointer to a member of reader to be used to fill the field if required
  /// @return a reference to the actual data
  template<typename Reader>
  inline const T& value(const Reader &reader, void (Reader::*func)(T&) const) const;
  
  /// @brief access the data, read on-demand
  /// @details On the first request and whenever is necessary, this method reads the data 
  /// using Reader::operator(), which must accept non-const reference to the type T.
  /// Reader is a type of an object function, which can fill this field
  /// with the appropriate information (i.e. read it).   
  /// @param[in] reader an object which has a method able to fill this field
  /// @return a reference to the actual data
  template<typename Reader>
  inline const T& value(Reader reader) const;
   
  /// @brief invalidate the field
  void inline invalidate() const throw() { itsChangedFlag=true; }

  /// @brief test validity
  /// @details To avoid unnecessary checks/duplicated invalidation of the field
  /// it is convenient to be able to test whether the field is still valid.
  /// Otherwise, any additional checks are pointless.
  /// @return true if the cache is valid
  bool inline isValid() const throw() { return !itsChangedFlag;}
  
private:
  /// true, if the field needs reading
  mutable bool itsChangedFlag;

  /// cached buffer
  mutable T itsValue;
};

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
