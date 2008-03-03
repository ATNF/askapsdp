/// @file
/// 
/// @brief This file contains tests for IndexedLess predicate
/// @details Test includes sorting a vector with index information.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef MAP_KEY_ITERATOR_TEST_H
#define MAP_KEY_ITERATOR_TEST_H

#include <askap/ConradError.h>
#include <askap/ConradUtil.h>
#include <askap/MapKeyIterator.h>

#include <cppunit/extensions/HelperMacros.h>


#include <map>
#include <string>
#include <vector>
#include <algorithm>

namespace conrad
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

} // namespace conrad

#endif // #ifndef MAP_KEY_ITERATOR_TEST_H
