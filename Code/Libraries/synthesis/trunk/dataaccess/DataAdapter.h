/// @file DataAdapter.h
///
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

namespace conrad {

namespace synthesis {

template<typename Iter, typename Sel, typename Inc>
class DataAdapter {
public:

  /// construct an adapter for the supplied iterator
  /// 
  /// @param[in] iter a const reference to the iterator to work with
  ///
  explicit DataAdapter(const Iter &iter) : mIter(iter) {}
  
  /// a default copy constructor is sufficient in this case
  
  /// access the required data. It is done by calling Sel::operator()
  /// for the iterator held inside this class
  ///
  /// @return the data requested (the type is determined by the selector)
  ///
  inline typename Sel::value_type operator*() const {
     return selector(mIter);
  }
  
  inline void operator++() const {
     incrementor(mIter);
  }
  
private:  
  Iter mIter;  /// SharedIter to work with. In fact, anything which can
               /// be passed to Inc::operator() (the default action is
	       /// to call the operator++) and whose operator* returns a
	       /// type accepted by Sel::operator(), which in turn returns
	       /// a DataAccessor. Comparison operators should also be
	       /// defined.
  Sel selector; /// the operator() of this object is called with the
                /// underlying iterator passed as a parameter in the
		/// access operator. It must return Sel::value_type
  Inc incrementor; /// the operator() of this object is called with the
                 /// underlying iterator passed as a parameter in the
		 /// increment operator. Return type doesn't matter (and
		 /// can be void).
};

/// two basic incrementors: one doing nothing (if one wants to write to the
/// same iterator) and another doing normal increments (if the destination
/// is pointed by a separate iterators)

struct NoIncrement {
   template<typename Iter>
   inline void operator()(const Iter &iter) const
   {}
};

struct Incremented {
   template<typename Iter>
   inline void operator()(const Iter &iter) const
   {
     ++iter;
   }
};

/// Data selectors
struct VisibilitySelector {
   typedef casa::Cube<casa::Complex>& value_type;
   template<typename Iter>
   inline value_type operator()(const Iter &iter) const
   {
     return iter->visibility();
   }
};

template<int N> struct BufferSelector {
     typedef casa::Cube<casa::Complex>& value_type;
     template<typename Iter>
     inline value_type operator()(const Iter &iter) const
     {
       return iter->visibility();
     }
  };


} // namespace synthesis

} // namespace conrad

#endif // #ifndef I_DATA_ADAPTER_H
