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

#ifndef FIXED_SIZE_CACHE_TCC
#define FIXED_SIZE_CACHE_TCC

namespace askap {

namespace scimath {

/// @brief construct the cache handler
/// @details
/// @param[in] size size of the cache (maximum number of cached elements)
template<typename Key, typename C>
FixedSizeCache<Key,C>::FixedSizeCache<Key,C>(size_t size) : itsCache(size), itsKeys(size),
           itsActiveElement(size), itsOldestElement(0), itsAllFilled(false)
{
   ASKAPDEBUGASSERT(size>0);
}           
   
/// @brief search by a key, create a new element if necessary
/// @details This method does a search. If the appropriate key is found, it is made active.
/// Otherwise a blank entry (with an uninitialised shared pointer) is created and made active.
/// @param[in] key a key to search for
template<typename Key, typename C>
void FixedSizeCache<Key,C>::find(const Key &key)
{
   itsNewElement = true;
   for (size_t el = 0; el<itsKeys.size(); ++el) {
        const int index = int(itsOldestElement) - el - 1;
        if (index<0) {
            // wrap around the end of the vector
            index += int(itsKeys.size());
            if (!itsAllFilled) {
                // these indices are not yet initialised
                break;
            }
        }
        ASKAPDEBUGASSERT((index>=0) && (index<int(itsKeys.size())));
        if (itsKeys[index] == key) {
            itsActiveElement = size_t(index);
            itsNewElement = false;
            return;
        }
   }
   // not found, oldest element has to be replaced
   itsActiveElement = itsOldestElement++;
   itsKeys[itsActiveElement] = key;
   ASKAPDEBUGASSERT(itsActiveElement<itsKeys.size());
   // strictly speaking we don't need to reset shared pointer, because it is supposed to be
   // reset by the user.
   itsCache[itsActiveElement].reset(); 
   // figure out the next oldest element
   if (itsOldestElement >= itsKeys.size()) {
       // wrap around the end of the vector
       itsOldestElement = 0;
       itsAllFilled = true;
   }   
}

/// @brief reset cache, remove all cached items
/// @details Sometimes, it may be neccessary to remove a reference on all elements in the cache
/// explicitly (i.e. to force destructors to run). This method brings the template back to 
/// the state it can be found just after construction.
template<typename Key, typename C>
void FixedSizeCache<Key,C>::reset()
{
   for (size_t index = 0; index < (itsAllFilled ? itsCache.size() : itsOldestElement); ++index) {
        ASKAPDEBUGASSERT(index<itsCache.size());
        itsCache[index].reset();
        itsKeys[index] = Key();
   }
   itsAllFilled = false;
   itsOldestElement = 0;
   itsActiveElement = itsCache.size(); // just to have something invalid
}

} // namespace scimath

} // namespace askap

#endif // #ifndef FIXED_SIZE_CACHE_TCC
