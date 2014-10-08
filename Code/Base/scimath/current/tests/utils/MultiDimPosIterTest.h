/// @file
/// 
/// @brief Unit tests for MultiDimPosIter.
/// @details MultiDimArrayPlaneIter is an iterator class designed to facilitate processing
/// of hypercube inside the solver.
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

#ifndef ASKAP_SCIMATH_MULTI_DIM_POS_ITER_TEST_H
#define ASKAP_SCIMATH_MULTI_DIM_POS_ITER_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <utils/MultiDimPosIter.h>
#include <askap/AskapUtil.h>

#include <vector>

namespace askap {

namespace scimath {

   class MultiDimPosIterTest : public CppUnit::TestFixture
    {
           CPPUNIT_TEST_SUITE(MultiDimPosIterTest);
           CPPUNIT_TEST(testFullDim);
           CPPUNIT_TEST(testEmpty);
           CPPUNIT_TEST(testRange);
           CPPUNIT_TEST(testIncompleteRange);
           CPPUNIT_TEST(testSplit);
           CPPUNIT_TEST(testUnbalancedSplit);
           CPPUNIT_TEST_EXCEPTION(testStartAfterEnd, AskapError);           
           CPPUNIT_TEST_EXCEPTION(testStartAfterEnd1, AskapError);           
           CPPUNIT_TEST_SUITE_END();
       public:
           void testFullDim() {
               MultiDimPosIter it(casa::IPosition(2,3,5));
               doTestFullDim(it);
               MultiDimPosIter it2;
               it2.init(casa::IPosition(2,3,5));
               doTestFullDim(it2);               
           }
           
           void doTestFullDim(MultiDimPosIter &it) {    
               CPPUNIT_ASSERT(it.hasMore());
               CPPUNIT_ASSERT(it.cursor().isEqual(casa::IPosition(2,0)));
               for (int y = 0; y < 5; ++y) {
                    // first index is the fastest to change
                    for (int x = 0; x < 3; ++x, it.next()) {
                         CPPUNIT_ASSERT(it.cursor().isEqual(casa::IPosition(2,x,y)));                        
                         CPPUNIT_ASSERT(it.hasMore());               
                    }
               }
               CPPUNIT_ASSERT(!it.hasMore());
               it.origin();
               CPPUNIT_ASSERT(it.cursor().isEqual(casa::IPosition(2,0)));
               CPPUNIT_ASSERT(it.hasMore());                                             
           }
           
           void testEmpty() {
               MultiDimPosIter it;
               CPPUNIT_ASSERT(!it.hasMore());                              
               it.origin();
               CPPUNIT_ASSERT(!it.hasMore());               
           }

           void testRange() {
               MultiDimPosIter it(casa::IPosition(2,4,6), casa::IPosition(2,1,2), casa::IPosition(2,3,5));
               doTestRange(it);
               MultiDimPosIter it2;
               it2.init(casa::IPosition(2,4,6), casa::IPosition(2,1,2), casa::IPosition(2,3,5));
               doTestRange(it2);                      
           }
           
           void doTestRange(MultiDimPosIter &it) {
               CPPUNIT_ASSERT(it.hasMore());
               CPPUNIT_ASSERT(it.cursor().isEqual(casa::IPosition(2,1,2)));
               for (int y = 2; y <= 5; ++y) {
                    // first index is the fastest to change
                    for (int x = (y == 2 ? 1 : 0); x <= 3; ++x, it.next()) {
                         CPPUNIT_ASSERT(it.cursor().isEqual(casa::IPosition(2,x,y)));                        
                         CPPUNIT_ASSERT(it.hasMore());               
                    }
               }
               CPPUNIT_ASSERT(!it.hasMore());
               it.origin();
               CPPUNIT_ASSERT(it.cursor().isEqual(casa::IPosition(2,1,2)));
               CPPUNIT_ASSERT(it.hasMore());                  
           }
           
           void testIncompleteRange() {
              MultiDimPosIter it(casa::IPosition(2,3,5), casa::IPosition(2,1,0), casa::IPosition(2,0,1));
              CPPUNIT_ASSERT(it.hasMore());
              CPPUNIT_ASSERT(it.cursor().isEqual(casa::IPosition(2,1,0)));
              it.next();
              CPPUNIT_ASSERT(it.hasMore());
              CPPUNIT_ASSERT(it.cursor().isEqual(casa::IPosition(2,2,0)));
              it.next();
              CPPUNIT_ASSERT(it.hasMore());
              CPPUNIT_ASSERT(it.cursor().isEqual(casa::IPosition(2,0,1)));
              it.next();
              CPPUNIT_ASSERT(!it.hasMore());               
           }
           
           void doTestSplit(casa::uInt nX, casa::uInt nY, casa::uInt nChunks) {
               std::vector<casa::IPosition> results;
               results.reserve(nX * nY);
               for (int y = 0; y < static_cast<int>(nY); ++y) {
                    // first index is the fastest to change
                    for (int x = 0; x < static_cast<int>(nX); ++x) {
                         results.push_back(casa::IPosition(2,x,y));
                    }
               }
               casa::uInt count = 0;
               for (casa::uInt chunk = 0; chunk < nChunks; ++chunk) {
                    MultiDimPosIter it;
                    for (it.init(casa::IPosition(2,static_cast<int>(nX),static_cast<int>(nY)),nChunks,chunk); it.hasMore(); it.next(),++count) {
                         CPPUNIT_ASSERT(count < results.size());
                         CPPUNIT_ASSERT(it.cursor().isEqual(results[count]));                                                 
                    }
               }
               CPPUNIT_ASSERT_EQUAL(results.size(), static_cast<size_t>(count));                                          
           }
           
           void testSplit() {
               doTestSplit(3,5,4);
           }
           
           void testUnbalancedSplit() {
               doTestSplit(9,304,216);
               doTestSplit(9,304,113);
               doTestSplit(9,304,177);
           }
           
           void testStartAfterEnd() {
               // the following should throw AskapError because start is beyond end
               MultiDimPosIter it(casa::IPosition(2,3,5), casa::IPosition(2,0,2), casa::IPosition(2,2,0));               
           }
           
           void testStartAfterEnd1() {
               // the following should throw AskapError because start is beyond end
               MultiDimPosIter it;
               // the following should throw AskapError because start is beyond end
               it.init(casa::IPosition(2,3,5), casa::IPosition(2,0,2), casa::IPosition(2,2,0));               
           }
    };
    
} // namespace scimath

} // namespace askap    
    
#endif // #ifndef ASKAP_SCIMATH_MULTI_DIM_POS_ITER_TEST_H



