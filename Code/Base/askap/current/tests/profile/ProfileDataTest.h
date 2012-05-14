/// @file
///
/// @brief This file contains tests for ProfileData
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
/// @author Max Voronkov <Maxim.Voronkov@csiro.au>

#ifndef ASKAP_PROFILE_DATA_TEST_H
#define ASKAP_PROFILE_DATA_TEST_H

#include <cppunit/extensions/HelperMacros.h>

// Class under test
#include <profile/ProfileData.h>

namespace askap {

class ProfileDataTest : public CppUnit::TestFixture {

        CPPUNIT_TEST_SUITE(ProfileDataTest);
        CPPUNIT_TEST(testCreate);
        CPPUNIT_TEST(testAdd);
        CPPUNIT_TEST_SUITE_END();
    public:
        void testCreate() {
           ProfileData pd;
           CPPUNIT_ASSERT_EQUAL(0l, pd.count());
           CPPUNIT_ASSERT_DOUBLES_EQUAL(0., pd.totalTime(),1e-6);           
           ProfileData pd2(3.3);
           CPPUNIT_ASSERT_EQUAL(1l, pd2.count());
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, pd2.totalTime(),1e-6);           
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, pd2.maxTime(),1e-6);           
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, pd2.minTime(),1e-6);                      
        }
        
        void testAdd() {
           ProfileData pd(3.3);
           CPPUNIT_ASSERT_EQUAL(1l, pd.count());
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, pd.totalTime(),1e-6);           
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, pd.maxTime(),1e-6);           
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, pd.minTime(),1e-6);                      
           pd.add(5.5);
           CPPUNIT_ASSERT_EQUAL(2l, pd.count());
           CPPUNIT_ASSERT_DOUBLES_EQUAL(8.8, pd.totalTime(),1e-6);           
           CPPUNIT_ASSERT_DOUBLES_EQUAL(5.5, pd.maxTime(),1e-6);           
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, pd.minTime(),1e-6);                      
           pd.add(0.7);
           CPPUNIT_ASSERT_EQUAL(3l, pd.count());
           CPPUNIT_ASSERT_DOUBLES_EQUAL(9.5, pd.totalTime(),1e-6);           
           CPPUNIT_ASSERT_DOUBLES_EQUAL(5.5, pd.maxTime(),1e-6);           
           CPPUNIT_ASSERT_DOUBLES_EQUAL(0.7, pd.minTime(),1e-6);                                 
           // copy
           ProfileData pd2(pd);
           pd.add(6.0);
           CPPUNIT_ASSERT_EQUAL(4l, pd.count());
           CPPUNIT_ASSERT_DOUBLES_EQUAL(15.5, pd.totalTime(),1e-6);           
           CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, pd.maxTime(),1e-6);           
           CPPUNIT_ASSERT_DOUBLES_EQUAL(0.7, pd.minTime(),1e-6);                                 
           CPPUNIT_ASSERT_EQUAL(3l, pd2.count());
           CPPUNIT_ASSERT_DOUBLES_EQUAL(9.5, pd2.totalTime(),1e-6);           
           CPPUNIT_ASSERT_DOUBLES_EQUAL(5.5, pd2.maxTime(),1e-6);           
           CPPUNIT_ASSERT_DOUBLES_EQUAL(0.7, pd2.minTime(),1e-6);                                            
        }
};
    
} // namespace askap

#endif // #ifndef ASKAP_PROFILE_DATA_TEST_H



