/// @file
/// @brief a single cached field of the data accessor 
///
/// @details TableConstDataAccessor manages a number of cached fields.
/// This class represents a single such field
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef CACHED_ACCESSOR_FIELD_TCC
#define CACHED_ACCESSOR_FIELD_TCC

/// @brief a single cached field of the data accessor 
///
/// @details TableConstDataAccessor manages a number of cached fields.
/// This class represents a single such field
/// Template parameters:
/// @li T is a type of the field
/// @li Reader is a type of an object function, which can fill this field
/// with the appropriate information (i.e. read it)
template<typename T>
struct CachedAccessorField {
  /// @brief initialize the class, set the flag that reading is required
  CachedAccessorField() : itsChangedFlag(true) {}
  
  /// @return a reference to the actual data
  operator const T&() const;

  /// @brief invalidate the field
  void invalidate() throw() { itsChangedFlag=true; }

private:
  /// true, if the field needs reading
  mutable bool itsChangedFlag;

  /// cached buffer
  mutable T itsValue;
};

#endif // #ifndef CACHED_ACCESSOR_FIELD_TCC
