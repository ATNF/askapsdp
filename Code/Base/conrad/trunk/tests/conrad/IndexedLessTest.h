/// @file
/// 
/// @brief This file contains tests for IndexedLess predicate
/// @details Test includes sorting a vector with index information.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef INDEXED_LESS_TEST_H
#define INDEXED_LESS_TEST_H

#include <conrad/ConradError.h>
#include <conrad/ConradUtil.h>
#include <conrad/IndexedLess.h>

#include <cppunit/extensions/HelperMacros.h>

#include <cmath>

#include <vector>
#include <algorithm>

namespace conrad
{

namespace utility
{

  class IndexedLessTest : public CppUnit::TestFixture {

     CPPUNIT_TEST_SUITE(IndexedLessTest);
     CPPUNIT_TEST(testSorting);
     CPPUNIT_TEST_SUITE_END();

  public:
     
     void testSorting() {
         const size_t nElements = 5;
         const double buf[nElements] = {1., 2.3, -5., 4.1, 0.7};
         std::vector<double> values(nElements);
         std::vector<size_t> indices(nElements);
         for (size_t i=0; i<nElements; ++i) {
              values[i] = buf[i];
              indices[i] = i;
         }
         std::sort(indices.begin(),indices.end(),indexedLess<size_t>(values.begin()));
         const size_t expectations[nElements] = {2, 4, 0, 1, 3};
         for (size_t i=0; i<nElements; ++i) {
              CPPUNIT_ASSERT(expectations[i] == indices[i]);
         }
     }
     
};

} // namespace utility

} // namespace conrad

#endif // #ifndef INDEXED_LESS_TEST_H
