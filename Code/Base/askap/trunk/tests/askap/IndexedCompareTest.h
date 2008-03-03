/// @file
/// 
/// @brief This file contains tests for IndexedLess predicate
/// @details Test includes sorting a vector with index information.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef INDEXED_COMPARE_TEST_H
#define INDEXED_COMPARE_TEST_H

#include <askap/ConradError.h>
#include <askap/ConradUtil.h>
#include <askap/IndexedCompare.h>

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
     CPPUNIT_TEST(testSortingLess);
     CPPUNIT_TEST(testSortingGreater);
     CPPUNIT_TEST(testSortingWithEquals);
     CPPUNIT_TEST_SUITE_END();

  public:
     
     void testSortingLess() {
         const size_t nElements = 5;
         const double buf[nElements] = {1., 2.3, -5., 4.1, 0.7};
         std::vector<double> values(nElements);
         std::vector<size_t> indices(nElements);
         for (size_t i=0; i<nElements; ++i) {
              values[i] = buf[i];
              indices[i] = i;
         }
         std::sort(indices.begin(),indices.end(),indexedCompare<size_t>(values.begin()));
         const size_t expectations[nElements] = {2, 4, 0, 1, 3};
         for (size_t i=0; i<nElements; ++i) {
              CPPUNIT_ASSERT(expectations[i] == indices[i]);
         }
     }
     
     void testSortingGreater() {
         const size_t nElements = 5;
         const double buf[nElements] = {1., 2.3, -5., 4.1, 0.7};
         std::vector<double> values(nElements);
         std::vector<size_t> indices(nElements);
         for (size_t i=0; i<nElements; ++i) {
              values[i] = buf[i];
              indices[i] = i;
         }
         std::sort(indices.begin(),indices.end(),
                   indexedCompare<size_t>(values.begin(),std::greater<double>()));
         const size_t expectations[nElements] = {3, 1, 0, 4, 2};
         for (size_t i=0; i<nElements; ++i) {
              CPPUNIT_ASSERT(expectations[i] == indices[i]);
         }
     }
     
     void testSortingWithEquals() {
         const size_t nEl = 180;
         double Vals[nEl]={2640,1.91999220432645e-13,1290,1290,1290,1290,
                    1290,1290,1290,1290,1290,
                    1290,1290,1290,1290,1290,
                    1290,1290,1289.99999999999,1290,1290,
                    1290,1290,1290,1289.99999999999,1290,
                    1290,1290,1290,1290,1290,
                    1290,1290,1290,1290,1290,
                    1290,1290,1290,1290,1290,
                    1290,1290,1290,1290,1290,
                    1350.00000000001,1350,1350,1350,1350,
                    1350,1350,1350,1350,1350,
                    1350,1350,1350,1350,1350,
                    1350,1350,1350,1350,1350,
                    1350,1350,1350,1350,1350,
                    1350,1350,1350,1350,1350,
                    1350,1350,1350,1350,1350,
                    1350,1350,1350,1350,1350,
                    1350,1350,1350,1350,2640,
                    1.91999220432645e-13,1290,1290,1290,1290,
                    1290,1290,1290,1290,1290,
                    1290,1290,1290,1290,1290,
                    1290,1290,1289.99999999999,1290,1290,
                    1290,1290,1290,1289.99999999999,1290,
                    1290,1290,1290,1290,1290,
                    1290,1290,1290,1290,1290,
                    1290,1290,1290,1290,1290,
                    1290,1290,1290,1290,1290,
                    1350.00000000001,1350,1350,1350,1350,
                    1350,1350,1350,1350,1350,
                    1350,1350,1350,1350,1350,
                    1350,1350,1350,1350,1350,
                    1350,1350,1350,1350,1350,
                    1350,1350,1350,1350,1350,
                    1350,1350,1350,1350,1350,
                    1350,1350,1350,1350,1350,
                    1350,1350,1350,1350};
         
         std::vector<double> values(nEl);
         std::vector<size_t> indices(nEl);
         for (size_t i=0;i<nEl;++i) {
              indices[i]=i;
              values[i]=Vals[i];
         }  
         std::sort(indices.begin(),indices.end(),
                  indexedCompare<size_t>(values.begin(),std::greater<double>()));
         // order of equal indices may be arbitrary (even depending on the 
         // compiler as the algorithm of sort was changed at some stage)
         CPPUNIT_ASSERT(indices[1] == 0); // second largest is only one
         
         for (size_t i=1;i<nEl;++i) {
              CPPUNIT_ASSERT(values[indices[i]] <= values[indices[i-1]]);
         }         
     }
};

} // namespace utility

} // namespace conrad

#endif // #ifndef INDEXED_COMPARE_TEST_H
