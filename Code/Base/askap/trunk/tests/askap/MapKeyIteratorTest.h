/// @file
/// 
/// @brief This file contains tests for IndexedLess predicate
/// @details Test includes sorting a vector with index information.
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

#ifndef MAP_KEY_ITERATOR_TEST_H
#define MAP_KEY_ITERATOR_TEST_H

#include <askap/AskapError.h>
#include <askap/AskapUtil.h>
#include <askap/MapKeyIterator.h>

#include <cppunit/extensions/HelperMacros.h>


#include <map>
#include <string>
#include <vector>
#include <algorithm>

namespace askap
{

namespace utility
{

  class MapKeyIteratorTest : public CppUnit::TestFixture {

     CPPUNIT_TEST_SUITE(MapKeyIteratorTest);
     CPPUNIT_TEST(testIterator);
     CPPUNIT_TEST_SUITE_END();

  public:
     
     void testIterator() {
          std::map<std::string, int> testMap;
          testMap["par1"]=1;
          testMap["par2"]=2;
          testMap["par3"]=3;
          testMap["par4"]=4;
          
          MapKeyIterator<std::map<std::string,int>::const_iterator> 
                                 it(testMap.begin());
          
          const MapKeyIterator<std::map<std::string,int>::const_iterator> 
                                 end(testMap.end());
          CPPUNIT_ASSERT(*it == "par1"); 
          ++it;
          CPPUNIT_ASSERT(*(it++) == "par2"); 
          CPPUNIT_ASSERT(*it == "par3"); 
          CPPUNIT_ASSERT(*(++it) == "par4"); 
          bool endFlag = (it != 
             MapKeyIterator<std::map<std::string,int>::const_iterator>(testMap.end()));
          CPPUNIT_ASSERT(endFlag);
          ++it;
          endFlag = (it == 
             MapKeyIterator<std::map<std::string,int>::const_iterator>(testMap.end()));
          CPPUNIT_ASSERT(endFlag);
          
          std::vector<std::string> parameters;
          std::copy(mapKeyBegin(testMap),mapKeyEnd(testMap),
             std::back_insert_iterator<std::vector<std::string> >(parameters));
          CPPUNIT_ASSERT(parameters.size()==4);
          CPPUNIT_ASSERT(parameters[0] == "par1");
          CPPUNIT_ASSERT(parameters[1] == "par2");
          CPPUNIT_ASSERT(parameters[2] == "par3");
          CPPUNIT_ASSERT(parameters[3] == "par4");
     }
     
};

} // namespace utility

} // namespace askap

#endif // #ifndef MAP_KEY_ITERATOR_TEST_H
