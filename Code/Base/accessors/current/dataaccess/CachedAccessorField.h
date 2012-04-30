/// @file
/// @brief a single cached field of the data accessor 
///
/// @details TableConstDataAccessor manages a number of cached fields.
/// This class represents a single such field
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef CACHED_ACCESSOR_FIELD_H
#define CACHED_ACCESSOR_FIELD_H

#include <askap/AskapError.h>

// boost includes
#ifdef _OPENMP
#include <boost/thread/shared_mutex.hpp>
#endif

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
struct CachedAccessorField  {
  /// @brief initialize the class, set the flag that reading is required
  CachedAccessorField() : itsChangedFlag(true), itsFlushFlag(false) {}
  
  /// @brief copy constructor
  /// @param[in] other an object to copy from
  /// @note reference semantics for casa arrays, but we're not copying this class where T is a casa array type. 
#ifdef _OPENMP
  CachedAccessorField(const CachedAccessorField<T> &other);
#else
  CachedAccessorField(const CachedAccessorField<T> &other) : itsChangedFlag(other.itsChangedFlag),
        itsFlushFlag(other.itsFlushFlag), itsValue(other.itsValue) {}
#endif
        
  /// @brief assignment operator
  /// @param[in] other an object to copy from
  /// @note reference semantics for casa arrays, but we're not using this method where T is a casa array type. 
  CachedAccessorField<T>& operator=(const CachedAccessorField<T> &other);
  
  /// @brief access the data, read on-demand
  /// @details On the first request and whenever is necessary, this method reads the data using 
  /// a given member of the Reader class.
  /// Reader is a type of an object function, which can fill this field
  /// with the appropriate information (i.e. read it). 
  /// @param[in] reader an object which has a method able to fill this field
  /// @param[in] func a pointer to a member of reader to be used to fill the field if required
  /// @return a reference to the actual data
  /// @note an exception is thrown if the read operation is required while the cache needs flush
  template<typename Reader>
  inline const T& value(const Reader &reader, void (Reader::*func)(T&) const) const;
  
  /// @brief access the data, read on-demand
  /// @details On the first request and whenever is necessary, this method reads the data 
  /// using Reader::operator(), which must accept non-const reference to the type T.
  /// Reader is a type of an object function, which can fill this field
  /// with the appropriate information (i.e. read it).   
  /// @param[in] reader an object which has a method able to fill this field
  /// @return a reference to the actual data
  /// @note an exception is thrown if the read operation is required while the cache needs flush
  template<typename Reader>
  inline const T& value(Reader reader) const;
  
  /// @brief access the data, throw exception if read is required
  /// @details When managing flush of the writable cache, it is handy to access the cache
  /// directly when one knows that no reading is required. The method without arguments
  /// throws exception if read on-demand is needed and returns the const-reference if the 
  /// field is up to date.
  /// @return a const reference to the actual data
  inline const T& value() const;
  
  /// @brief access the data for writing following read on-demand
  /// @details Unlike the corresponding value method, this one returns
  /// non-const reference which allows modifications after the read
  /// on demand is completed.
  /// @param[in] reader an object which has a method able to fill this field
  /// @param[in] func a pointer to a member of reader to be used to fill the field if required
  /// @return a reference to the actual data
  /// @note an exception is thrown if the read operation is required while the cache needs flush
  template<typename Reader>
  inline T& rwValue(const Reader &reader, void (Reader::*func)(T&) const);
  
  /// @brief access the data for writing following read on-demand
  /// @details Unlike the corresponding value method, this one returns
  /// non-const reference which allows modifications after the read
  /// on demand is completed.
  /// @param[in] reader an object which has a method able to fill this field
  /// @return a reference to the actual data
  /// @note an exception is thrown if the read operation is required while the cache needs flush
  template<typename Reader>
  inline T& rwValue(Reader reader);
  
  /// @brief access the data for writing without read
  /// @details An exception is thrown if the read operation is required
  /// @return a reference to the actual data
  inline T& rwValue() const;
     
  /// @brief invalidate the field
  void inline invalidate() const throw();

  /// @brief test validity
  /// @details To avoid unnecessary checks/duplicated invalidation of the field
  /// it is convenient to be able to test whether the field is still valid.
  /// Otherwise, any additional checks are pointless.
  /// @return true if the cache is valid
  bool inline isValid() const throw() { return !isChanged();}
  
  /// @brief test whether any write operation took place
  /// @details The interface supports write operation (i.e. non-const
  /// reference can be obtained). This method tests whether cache needs to be 
  /// flushed.
  /// @return true, if non-const reference had been obtained at least once
  bool inline flushNeeded() const throw() { return itsFlushFlag; }
  
  /// @brief notify that this field had been synchronised
  void inline flushed() const throw() { itsFlushFlag = false; }
protected:
  /// @brief helper method to check if the cache needs an update
  /// @details This method has been introduced to provide better encapsulation of
  /// the synchronisation code if thread safety is required
  /// @return true, if the cache needs update
  bool isChanged() const;
private:
  /// @brief true, if the field needs reading
  mutable bool itsChangedFlag;
  
  /// @brief true, if there was a write operation
  mutable bool itsFlushFlag;

  /// @brief cached buffer
  mutable T itsValue;
  
#ifdef _OPENMP
  /// @brief mutex for synchronisation
  mutable boost::shared_mutex itsMutex;
#endif
};

} // namespace accessors

} // namespace askap

#include <dataaccess/CachedAccessorField.tcc>

#endif // #ifndef CACHED_ACCESSOR_FIELD_H

