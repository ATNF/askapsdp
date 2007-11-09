/// @file
/// 
/// @brief A binary predicate to compare two numbers referred by indices 
/// @details While sorting a vector, it is often necessary to track
/// permutations. One way of doing this is to write a std::pair-like class
/// with comparison operators using one element of the pair only and store
/// both value and its index. This is done in the PairOrderedFirst class 
/// in AIPS++, which I wrote some time ago. However, there exists a more elegant
/// solution using a version of std::sort with a user-suppled binary predicate.
/// This file defines such a binary predicate class, which compares two values
/// stored in a container defined by its random access iterator of the origin
/// each time it is asked to compare two indicies. Each instance of the class
/// holds a copy of the rangom access iterator.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef INDEXED_LESS_H
#define INDEXED_LESS_H

#include <functional>

namespace conrad {

namespace utility {

/// @brief A binary predicate to compare two numbers referred by indices 
/// @details While sorting a vector, it is often necessary to track
/// permutations. One way of doing this is to write a std::pair-like class
/// with comparison operators using one element of the pair only and store
/// both value and its index. This is done in the PairOrderedFirst class 
/// in AIPS++, which I wrote some time ago. However, there exists a more elegant
/// solution using a version of std::sort with a user-suppled binary predicate.
/// This is such a binary predicate class, which compares two values
/// stored in a container defined by its random access iterator of the origin
/// each time it is asked to compare two indicies. Each instance of the class
/// holds a copy of the rangom access iterator.
template<typename Iter, typename IndexType>
struct IndexedLess : public std::binary_function<IndexType,IndexType,bool> {
   /// @brief constructor
   /// @details
   /// @param[in] iter random access iterator to work with
   IndexedLess(const Iter &iter) : itsIter(iter) {}
   
   /// @brief main operator of the predicate
   /// @details Returns true if the value referred to by the first index
   /// is less than the value referred to by the second index
   /// @param[in] index1 index of the first value
   /// @param[in] index2 index of the second value
   /// @return true if value[index1]<value[index2]
   bool operator()(const IndexType &index1, const IndexType &index2) const 
   {
      return itsComparator(*(itsIter+index1), *(itsIter+index2)); 
   }
   
   
private:
   /// random access iterator to work with
   Iter itsIter;
   /// underlying binary predicate to do the comparison
   std::less<typename Iter::value_type> itsComparator;
};

/// @brief helper function to construct IndexedLess object
/// @details It is handy to have a helper method to avoid 
/// writing type names all the time. Function can extract the 
/// template parameter from the argument type, i.e. automatically
/// @param[in] iter random access iterator to work with
template<typename IndexType, typename Iter>
IndexedLess<Iter,IndexType> indexedLess(const Iter &iter) 
{
   return IndexedLess<Iter,IndexType>(iter);
}

} // namespace utility

} // namespace conrad

#endif // #define INDEXED_LESS_H

