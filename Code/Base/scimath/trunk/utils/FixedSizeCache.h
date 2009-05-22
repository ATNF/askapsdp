/// @file
/// 
/// @brief helper cache template: a map of a fixed size 
/// @details Cache of some object can be based on a maps of shared pointers. Sometimes,
/// we need to limit the number of elements in the cache to stop map from growing infinitely.
/// This template provides such a cache class.  
///
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

#ifndef FIXED_SIZE_CACHE_H
#define FIXED_SIZE_CACHE_H

#include <askap/AskapError.h>

#include <boost/shared_ptr.hpp>

#include <vector>

namespace askap {

namespace scimath {

/// @brief helper cache template: a map of a fixed size 
/// @details Cache of some object can be based on a maps of shared pointers. Sometimes,
/// we need to limit the number of elements in the cache to stop map from growing infinitely.
/// This template provides such a cache class.  
/// @todo This class uses many ideas from UVWMachineCache (in the synthesis/dataaccessor package),
/// which just has a composite key defined by two directions. It would be good to check at some
/// stage whether UVWMachineCache can be rewritten to be derived from this template and whether 
/// such rearrangement of the code is practical.
/// @ingroup utils
template<typename Key, typename C>
struct FixedSizeCache {
   /// @brief shared pointer type
   typedef typename boost::shared_ptr<C> ShPtr;
   
   /// @brief construct the cache handler
   /// @details
   /// @param[in] size size of the cache (maximum number of cached elements)
   explicit FixedSizeCache(size_t size);
   
   /// @brief search by a key, create a new element if necessary
   /// @details This method does a search. If the appropriate key is found, it is made active.
   /// Otherwise a blank entry (with an uninitialised shared pointer) is created and made active.
   /// @param[in] key a key to search for
   void find(const Key &key);
   
   /// @brief check whether this element is brand new
   /// @details This method returns true upon construction of the template and if the preceding 
   /// call to find didn't locate the existing item in the cache (i.e. the active element has
   /// to be initialised)
   /// @return true, if the currently active element is new (i.e. not found in the previous search)
   inline bool notFound() const { return itsNewElement;}
   
   /// @brief access to the active element
   /// @details
   /// @return a reference to the shared pointer
   inline ShPtr& cachedItem() { 
      ASKAPDEBUGASSERT(itsActiveElement<itsCache.size()); 
      return itsCache[itsActiveElement]; 
   }
   
   /// @brief const access to the active element
   /// @details
   /// @return a const reference to the shared pointer
   inline const ShPtr& cachedItem() const { 
      ASKAPDEBUGASSERT(itsActiveElement<itsCache.size()); 
      return itsCache[itsActiveElement]; 
   }
   
   /// @brief reset cache, remove all cached items
   /// @details Sometimes, it may be neccessary to remove a reference on all elements in the cache
   /// explicitly (i.e. to force destructors to run). This method brings the template back to 
   /// the state it can be found just after construction.
   void reset();

private:   
   /// @brief actual cache
   std::vector<ShPtr> itsCache;
   
   /// @brief vector of keys
   std::vector<Key> itsKeys;
   
   /// @brief index of the active element
   size_t itsActiveElement;
   
   /// @brief index of the oldest element in the cache
   size_t itsOldestElement;
   
   /// @brief true, if the active element is new
   /// @details We can't rely on the shared pointer being uninitialised because the users of
   /// this class may assign a special meaning to this state. Therefore, a separate flag is used.
   bool itsNewElement;   
   
   /// @brief true, if all elements of the cache are filled
   bool itsAllFilled;
}; 

} // namespace scimath

} // namespace askap

#include <utils/FixedSizeCache.tcc>

#endif // #ifndef FIXED_SIZE_CACHE_H

