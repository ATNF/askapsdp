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
           pt.notifyExit(5.0);
           std::map<std::string, ProfileData> stats;
           pt.extractStats(stats);
           CPPUNIT_ASSERT_EQUAL(size_t(2),stats.size());
           CPPUNIT_ASSERT(stats.find("root") != stats.end());
           CPPUNIT_ASSERT(stats.find("root.test") != stats.end());
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, stats["root.test"].totalTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, stats["root.test"].maxTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, stats["root.test"].minTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, stats["root"].totalTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, stats["root"].maxTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, stats["root"].minTime(),1e-6);

           std::map<std::string, ProfileData> globalStats;
           pt.extractStats(globalStats, false);
           CPPUNIT_ASSERT_EQUAL(size_t(2),globalStats.size());
           CPPUNIT_ASSERT(globalStats.find("::root") != globalStats.end());
           CPPUNIT_ASSERT(globalStats.find("test") != globalStats.end());
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, globalStats["test"].totalTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, globalStats["test"].maxTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, globalStats["test"].minTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, globalStats["::root"].totalTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, globalStats["::root"].maxTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, globalStats["::root"].minTime(),1e-6);           
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

           std::map<std::string, ProfileData> stats;
           pt.extractStats(stats);
           CPPUNIT_ASSERT_EQUAL(size_t(6),stats.size());
           CPPUNIT_ASSERT(stats.find("root") != stats.end());
           CPPUNIT_ASSERT(stats.find("root.test") != stats.end());
           CPPUNIT_ASSERT(stats.find("root.test.low_level_test") != stats.end());
           CPPUNIT_ASSERT(stats.find("root.test.low_level_test.another_test") != stats.end());
           CPPUNIT_ASSERT(stats.find("root.test.low_level_test.another_test.test") != stats.end());
           CPPUNIT_ASSERT(stats.find("root.test.low_level_test.another_test.fft") != stats.end());
           CPPUNIT_ASSERT_EQUAL(0l, stats["root"].count());
           CPPUNIT_ASSERT_DOUBLES_EQUAL(5.5, stats["root.test"].totalTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(5.5, stats["root.test"].maxTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(5.5, stats["root.test"].minTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(4.4, stats["root.test.low_level_test"].totalTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(4.4, stats["root.test.low_level_test"].maxTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(4.4, stats["root.test.low_level_test"].minTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, stats["root.test.low_level_test.another_test"].totalTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, stats["root.test.low_level_test.another_test"].maxTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, stats["root.test.low_level_test.another_test"].minTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(1e-3, stats["root.test.low_level_test.another_test.test"].totalTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(1e-3, stats["root.test.low_level_test.another_test.test"].maxTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(1e-3, stats["root.test.low_level_test.another_test.test"].minTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, stats["root.test.low_level_test.another_test.fft"].totalTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, stats["root.test.low_level_test.another_test.fft"].maxTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, stats["root.test.low_level_test.another_test.fft"].minTime(),1e-6);           

           std::map<std::string, ProfileData> globalStats;
           pt.extractStats(globalStats, false);
           CPPUNIT_ASSERT_EQUAL(size_t(5),globalStats.size());
           CPPUNIT_ASSERT(globalStats.find("::root") != globalStats.end());
           CPPUNIT_ASSERT(globalStats.find("test") != globalStats.end());
           CPPUNIT_ASSERT(globalStats.find("low_level_test") != globalStats.end());
           CPPUNIT_ASSERT(globalStats.find("another_test") != globalStats.end());
           CPPUNIT_ASSERT(globalStats.find("fft") != globalStats.end());
           CPPUNIT_ASSERT_DOUBLES_EQUAL(5.501, globalStats["test"].totalTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(5.5, globalStats["test"].maxTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(1e-3, globalStats["test"].minTime(),1e-6);
           CPPUNIT_ASSERT_EQUAL(2l, globalStats["test"].count());
           CPPUNIT_ASSERT_DOUBLES_EQUAL(4.4, globalStats["low_level_test"].totalTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(4.4, globalStats["low_level_test"].maxTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(4.4, globalStats["low_level_test"].minTime(),1e-6);
           CPPUNIT_ASSERT_EQUAL(1l, globalStats["low_level_test"].count());
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, globalStats["another_test"].totalTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, globalStats["another_test"].maxTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, globalStats["another_test"].minTime(),1e-6);
           CPPUNIT_ASSERT_EQUAL(1l, globalStats["another_test"].count());
           CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, globalStats["fft"].totalTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, globalStats["fft"].maxTime(),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, globalStats["fft"].minTime(),1e-6);
           CPPUNIT_ASSERT_EQUAL(1l, globalStats["fft"].count());
           
        }
 };
    
} // namespace askap

#endif // #ifndef ASKAP_PROFILE_TREE_TEST_H

