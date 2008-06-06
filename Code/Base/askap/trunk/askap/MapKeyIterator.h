/// @file
/// 
/// @brief Iterator over map keys
/// @details This is an adapter template which works in conjunction with the
/// std::map iterator or const_iterator. It behaves like a normal STL iterator
/// and refers to the map key (i.e. takes "first" field of the pair referenced
/// by the map iterator). In principle, any iterator with returning a pair
/// of values can be used with this adapter. The first element of the pair
/// is always used (this corresponds to the key of the map). This adapter is
/// used, e.g. if a list of parameters have to be build form maps storing some
/// data. 
///
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef MAP_KEY_ITERATOR_H
#define MAP_KEY_ITERATOR_H

#include <iterator>

namespace askap {

namespace utility {

/// @brief Iterator over map keys
/// @details This is an adapter template which works in conjunction with the
/// std::map iterator or const_iterator. It behaves like a normal STL iterator
/// and refers to the map key (i.e. takes "first" field of the pair referenced
/// by the map iterator). In principle, any iterator with returning a pair
/// of values can be used with this adapter. The first element of the pair
/// is always used (this corresponds to the key of the map). This adapter is
/// used, e.g. if a list of parameters have to be build form maps storing some
/// data. 
template<typename Iter> struct MapKeyIterator : public std::iterator<
                   typename std::iterator_traits<Iter>::iterator_category,
                   typename std::iterator_traits<Iter>::value_type::first_type,
                   typename std::iterator_traits<Iter>::difference_type,
                   typename std::iterator_traits<Iter>::value_type::first_type *,
                   typename std::iterator_traits<Iter>::value_type::first_type &> {

   /// @brief type of the value for this iterator
   typedef typename std::iterator_traits<Iter>::value_type::first_type value_type;

   /// @brief construct the adapter
   /// @details This method remembers given map iterator and works with it
   /// later.
   /// @param[in] iter iterator to work with
   inline MapKeyIterator(const Iter &iter) : itsIter(iter) {}
   
   /// @brief comparison operator
   /// @details It compares underlying map iterators and returns the result
   /// @param[in] other another iterator
   /// @return true if two iterators match
   bool inline operator==(const MapKeyIterator &other) const 
        { return itsIter == other.itsIter;}

   /// @brief comparison operator
   /// @details It compares underlying map iterators and returns the result
   /// @param[in] other another iterator
   /// @return true if two iterators differ
   bool inline operator!=(const MapKeyIterator &other) const 
        { return itsIter != other.itsIter;}
   
   /// @brief prefix increment
   /// @details This operator increments the underlying map iterator
   /// @return a reference to itself to be able to chain several operators
   inline MapKeyIterator<Iter>&  operator++();
   
   /// @brief postfix increment
   /// @details This operator increments the underlying map iterator
   /// @return a reference to itself to be able to chain several operators
   inline MapKeyIterator<Iter> operator++(int);
   
   /// @brief dereference operator
   /// @details This operator dereferences the underlying map iterator and
   /// returns the key field.
   /// @return key corresponding to the current iteration
   inline const value_type& operator*() const {return (*itsIter).first; }
   
private:
   /// @brief underlying map iterator 
   Iter itsIter;   
};

/// @brief prefix increment
/// @details This operator increments the underlying map iterator
/// @return a reference to itself to be able to chain several operators
template<typename Iter>
inline MapKeyIterator<Iter>&  MapKeyIterator<Iter>::operator++()
{
  ++itsIter;
  return *this;
}

/// @brief postfix increment
/// @details This operator increments the underlying map iterator
/// @return a reference to itself to be able to chain several operators
template<typename Iter>
inline MapKeyIterator<Iter> MapKeyIterator<Iter>::operator++(int)
{
  MapKeyIterator result(*this);
  ++itsIter;
  return result;
}

/// @brief helper method to create KeyMapIterator
/// @details This method is used to generate the appropriate template instance
/// automatically, based on the type of the argument
/// @param[in] cont a map-like container
template<typename Cont>
inline MapKeyIterator<typename Cont::const_iterator> mapKeyBegin(const Cont &cont) 
{
  return MapKeyIterator<typename Cont::const_iterator>(cont.begin());
} 

/// @brief helper method to create KeyMapIterator
/// @details This method is used to generate the appropriate template instance
/// automatically, based on the type of the argument
/// @param[in] cont a map-like container
template<typename Cont>
inline MapKeyIterator<typename Cont::const_iterator> mapKeyEnd(const Cont &cont) 
{
  return MapKeyIterator<typename Cont::const_iterator>(cont.end());
} 

} // namespace utility

} // namespace askap


#endif // #ifndef MAP_KEY_ITERATOR_H
