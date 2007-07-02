/// @file DataAdapter.h
/// @brief a helper template to be used with SharedIter
/// @details
/// DataAdapter: a helper template to be used in conjunction with the
///        SharedIter. It allows to call STL algorithms by selecting
///        the visibility data from the DataAccessor (i.e. the
///        operator* will return a reference to the visibility array,
///        instead of the whole data accessor). Optionally, it is
///        possible to ignore calls to the operator++. This allows to 
///        write to the same data accessor, which is currently read or
///        its associated buffers.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef I_DATA_ADAPTER_H
#define I_DATA_ADAPTER_H

#include <casa/Arrays/Cube.h>
#include <string>

namespace conrad {

namespace synthesis {

/// @brief A helper template to be used with SharedIter
/// @details
/// It allows to call STL algorithms by selecting
/// the visibility data from the DataAccessor (i.e. the
/// operator* will return a reference to the visibility array,
/// instead of the whole data accessor). Optionally, it is
/// possible to ignore calls to the operator++. This allows to 
/// write to the same data accessor, which is currently read or
/// its associated buffers.
/// @ingroup dataaccess
template<typename Iter, typename Sel, typename Inc>
class DataAdapter {
public:

  /// construct an adapter for the supplied iterator
  /// 
  /// @param[in] iter a const reference to the iterator to work with
  /// @param[in] selector a const reference to the selector object
  ///            choosing what field this adapter returns. This selector
  ///            is a different entity from the IDataSelector and related
  ///            classes, and is used with this template only.
  explicit DataAdapter(const Iter &iter, const Sel &selector = Sel()) :
                itsIter(iter), itsSelector(selector) {}
  
  /// a default copy constructor is sufficient in this case
  
  /// access the required data. It is done by calling Sel::operator()
  /// for the iterator held inside this class
  ///
  /// @return the data requested (the type is determined by the selector)
  ///
  inline typename Sel::value_type operator*() const {
     return itsSelector(itsIter);
  }

  /// increment operation
  inline void operator++() const {
     itsIncrementor(itsIter);
  }
  
private:  
  Iter itsIter;/// SharedIter to work with. In fact, anything which can
               /// be passed to Inc::operator() (the default action is
	       /// to call the operator++) and whose operator* returns a
	       /// type accepted by Sel::operator(), which in turn returns
	       /// a DataAccessor. Comparison operators should also be
	       /// defined.
  Sel itsSelector; /// the operator() of this object is called with the
                /// underlying iterator passed as a parameter in the
		/// access operator. It must return Sel::value_type
  Inc itsIncrementor; /// the operator() of this object is called with the
                 /// underlying iterator passed as a parameter in the
		 /// increment operator. Return type doesn't matter (and
		 /// can be void).
};


/// @brief an incrementor, which does nothing
/// @details
/// there are two basic incrementors: one doing nothing (if one wants
/// to write to the
/// same iterator) and another doing normal increments (if the destination
/// is pointed by a separate iterators)
struct NoIncrement {
   /// this operator of the given class does nothing
   /// @param[in] iter ignored
   template<typename Iter>
   inline void operator()(const Iter &iter) const
   {}
};

/// @brief an incrementor, which does normal increments
/// @details
/// there are two basic incrementors: one doing nothing (if one wants
/// to write to the
/// same iterator) and another doing normal increments (if the destination
/// is pointed by a separate iterators)
struct Incremented {
   /// @brief increment of the given iterator
   /// @details 
   /// @param[in] iter iterator to increment
   template<typename Iter>
   inline void operator()(const Iter &iter) const
   {
     ++iter;
   }
};

/// @brief data selector, which selects read-write visibility
struct VisibilitySelector {

   /// a type of the result
   typedef casa::Cube<casa::Complex>& value_type;

   /// @brief access to the visibility cube
   /// @details The method returns a non-const reference to visibilities
   /// @param[in] iter iterator to work with
   template<typename Iter>
   inline value_type operator()(const Iter &iter) const
   {
     return iter->rwVisibility();
   }
};

/// @brief data selector, which selects given buffer
struct BufferSelector {
  /// @brief set up the selector for a given buffer
  /// @details
  /// @param[in] buffer the name of the buffer
  BufferSelector(const std::string &buffer) : itsBufferName(buffer) {}

  /// a type of the result
  typedef casa::Cube<casa::Complex>& value_type;

  
  /// @brief access to the visibility cube
  /// @details The method returns a non-const reference to visibilities
  /// in the given buffer
  /// @param[in] iter iterator to work with
  template<typename Iter>
  inline value_type operator()(const Iter &iter) const
  {
    return iter.buffer(itsBufferName).rwVisibility();
  }  
private:
  /// buffer name to work with
  const std::string itsBufferName;
};

/// helper functions to construct a desired adapter without specifying
/// lots of template type arguments

/// VisAdapter - write access to the visibility data (use VisibilitySelector
///              described above)
///
/// template arguments:
/// Iter a type of the iterator (autodetected)
/// Inc  a type of the incrementor (autodetected)
/// method parameters:
/// @param[in] iter a valid SharedIter (with a permission to write)
/// the last dummy parameter is an incrementor object. It is used for type identification
///              only. An object of this type constructed with a default
///              constructor will be  used for actual increment of the
///              iterator passed in the first parameter.
///
/// Usage example:
///     transform(output_iter,output_iter.end(),
///          VisAdapter(output_iter,NoIncrement()));
///
template<typename Iter, typename Inc>
DataAdapter<Iter, VisibilitySelector, Inc> VisAdapter(Iter iter,
const Inc &) {
  return DataAdapter<Iter, VisibilitySelector, Inc>(iter);
}

/// the same as the previous function, but with a fixed incrementor
/// (which just calls the increment operator of the iterator).
/// To be used as a default version of the method (as the default
/// template parameters are not allowed for functions).
///
/// Usage example:
///     transform(input_iter,input_iter.end(),
///               VisAdapter(output_iter));
///
template<typename Iter>
DataAdapter<Iter, VisibilitySelector, Incremented> VisAdapter(Iter iter)
{
  return DataAdapter<Iter, VisibilitySelector, Incremented>(iter);
}

/// BufferAdapter - write access to a buffer, which name is known at
/// the compilation stage

/// template arguments:
/// Iter a type of the iterator (autodetected)
/// Inc  a type of the incrementor (autodetected)
/// method parameters:
/// @param[in] buffer a name of the buffer to use
/// @param[in] iter a valid SharedIter (with a permission to write)
/// the last dummy parameter is an incrementor object. It is used for type identification
///              only. An object of this type constructed with a default
///              constructor will be  used for actual increment of the
///              iterator passed in the first parameter.
///
/// Usage example:
///     transform(output_iter,output_iter.end(),
///          BufferAdapter("MODEL_DATA",output_iter,NoIncrement()));
///
template<int N, typename Iter, typename Inc>
DataAdapter<Iter, BufferSelector, Inc> BufferAdapter(const std::string &buffer,
                       Iter iter, const Inc &) {
  return DataAdapter<Iter, BufferSelector, Inc>(iter,BufferSelector(buffer));
}

/// the same as the previous function, but with a fixed incrementor
/// (which just calls the increment operator of the iterator).
/// To be used as a default version of the method (as the default
/// template parameters are not allowed for functions).
///
/// Usage example:
///     transform(input_iter,input_iter.end(),
///               BufferAdapter("MODEL_DATA",output_iter));
///
template<typename Iter>
DataAdapter<Iter, BufferSelector, Incremented>
            BufferAdapter(const std::string &buffer, Iter iter)
{
  return DataAdapter<Iter, BufferSelector, Incremented>(iter,
                           BufferSelector(buffer));
}

} // namespace synthesis

} // namespace conrad

#endif // #ifndef I_DATA_ADAPTER_H
