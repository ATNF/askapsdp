/// @file
/// 
/// @brief test of the helper cache template representing a map of a fixed size 
/// @details Cache of some object can be based on a maps of shared pointers. Sometimes,
/// we need to limit the number of elements in the cache to stop map from growing infinitely.
/// This tested template provides such a cache class.  
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

#ifndef FIXED_SIZE_CACHE_TEST_H
#define FIXED_SIZE_CACHE_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <boost/shared_ptr.hpp>
#include <utils/FixedSizeCache.h>


namespace askap {

namespace scimath {

class FixedSizeCacheTest : public CppUnit::TestFixture 
{
   CPPUNIT_TEST_SUITE(FixedSizeCacheTest);
   CPPUNIT_TEST(testSingleElement);
   CPPUNIT_TEST(testMultipleElements);
   CPPUNIT_TEST_SUITE_END();
public:
   void testSingleElement() {
      FixedSizeCache<string,int> cache(1);
      CPPUNIT_ASSERT(cache.notFound());      
      cache.find("1");
      CPPUNIT_ASSERT(cache.notFound());
      cache.cachedItem().reset(new int(5));
      CPPUNIT_ASSERT(cache.cachedItem());
      CPPUNIT_ASSERT(*cache.cachedItem() == 5);
      cache.find("1");
      CPPUNIT_ASSERT(!cache.notFound());      
      CPPUNIT_ASSERT(cache.cachedItem());
      CPPUNIT_ASSERT(*cache.cachedItem() == 5);
      cache.find("2");
      CPPUNIT_ASSERT(cache.notFound());
      cache.cachedItem().reset(new int(3));
      CPPUNIT_ASSERT(cache.cachedItem());
      CPPUNIT_ASSERT(*cache.cachedItem() == 3);
      cache.find("2");
      CPPUNIT_ASSERT(!cache.notFound());      
      CPPUNIT_ASSERT(cache.cachedItem());
      CPPUNIT_ASSERT(*cache.cachedItem() == 3);
      cache.find("1");
      CPPUNIT_ASSERT(cache.notFound());
      cache.cachedItem().reset(new int(5));      
      CPPUNIT_ASSERT(cache.cachedItem());
      CPPUNIT_ASSERT(*cache.cachedItem() == 5);
      cache.reset();
      cache.find("1");
      CPPUNIT_ASSERT(cache.notFound());        
      CPPUNIT_ASSERT(!cache.cachedItem());
   }
   void testMultipleElements() {
   }
}; 

} // namespace scimath

} // namespace askap

#endif // #ifndef FIXED_SIZE_CACHE_TEST_H


