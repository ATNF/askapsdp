/// @file
///
/// @brief This file contains tests for ProfileTree
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

#ifndef ASKAP_PROFILE_TREE_TEST_H
#define ASKAP_PROFILE_TREE_TEST_H

#include <cppunit/extensions/HelperMacros.h>

// Class under test
#include <profile/ProfileTree.h>

#include <askap/AskapError.h>

namespace askap {

class ProfileTreeTest : public CppUnit::TestFixture {

        CPPUNIT_TEST_SUITE(ProfileTreeTest);
        CPPUNIT_TEST(testCreate);
        CPPUNIT_TEST_EXCEPTION(testExitFromRoot,AskapError);
        CPPUNIT_TEST_EXCEPTION(testUnpairedExitAndEntry,AskapError);
        CPPUNIT_TEST(testRecursion);
        CPPUNIT_TEST_SUITE_END();
    public:
        void testCreate() {
           ProfileTree pt;
           CPPUNIT_ASSERT(pt.isRootCurrent());
           pt.notifyEntry("test");
           CPPUNIT_ASSERT(!pt.isRootCurrent());
           pt.notifyExit("test",3.3);
           CPPUNIT_ASSERT(pt.isRootCurrent());
        }
        
        void testExitFromRoot() {
           ProfileTree pt;
           CPPUNIT_ASSERT(pt.isRootCurrent());
           pt.notifyExit("root",3.3);
           CPPUNIT_ASSERT(pt.isRootCurrent());
        }
        
        void testUnpairedExitAndEntry() {
           ProfileTree pt;
           CPPUNIT_ASSERT(pt.isRootCurrent());
           pt.notifyEntry("test");
           CPPUNIT_ASSERT(!pt.isRootCurrent());
           pt.notifyExit("another_test",3.3);
           CPPUNIT_ASSERT(pt.isRootCurrent());        
        }
        
        void testRecursion() {
           ProfileTree pt;
           CPPUNIT_ASSERT(pt.isRootCurrent());
           pt.notifyEntry("test");
           CPPUNIT_ASSERT(!pt.isRootCurrent());
           pt.notifyEntry("low_level_test");
           pt.notifyEntry("another_test");
           pt.notifyEntry("test");
           CPPUNIT_ASSERT(!pt.isRootCurrent());
           pt.notifyExit("test",0.001);
           CPPUNIT_ASSERT(!pt.isRootCurrent());
           pt.notifyEntry("fft");
           CPPUNIT_ASSERT(!pt.isRootCurrent());
           pt.notifyExit("fft",1.0);                    
           CPPUNIT_ASSERT(!pt.isRootCurrent());
           pt.notifyExit("another_test",3.3);
           CPPUNIT_ASSERT(!pt.isRootCurrent());
           pt.notifyExit("low_level_test",4.4);
           CPPUNIT_ASSERT(!pt.isRootCurrent());
           pt.notifyExit("test",5.5);
           CPPUNIT_ASSERT(pt.isRootCurrent());            
        }
 };
    
} // namespace askap

#endif // #ifndef ASKAP_PROFILE_TREE_TEST_H

