/// @file SignalManagerTest.h
/// 
/// @brief This file contains tests for macros defined in AskapError.
/// @details The tests cover the ASKAPASSERT and ASKAPCHECK macros
///
/// @copyright (c) 2010 CSIRO
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
/// @author Ben Humphreys <Ben.Humphreys@csiro.au>

#ifndef ASKAP_SIGNAL_MANAGER_TEST_H
#define ASKAP_SIGNAL_MANAGER_TEST_H

// System includes
#include <sys/types.h>
#include <unistd.h>

#include <cppunit/extensions/HelperMacros.h>

#include "askap/SignalManagerSingleton.h"
#include "askap/SignalCounter.h"

namespace askap
{
  class SignalManagerTest : public CppUnit::TestFixture {
    
    CPPUNIT_TEST_SUITE(SignalManagerTest);
    CPPUNIT_TEST(testSingleton);
    CPPUNIT_TEST(testRegisterHandler);
    CPPUNIT_TEST(testRemoveHandler);
    CPPUNIT_TEST(testMultiple);
    CPPUNIT_TEST_SUITE_END();
    
  public:
    
    void testSingleton() {
        SignalManagerSingleton* instance = SignalManagerSingleton::instance();
        CPPUNIT_ASSERT(instance);
        CPPUNIT_ASSERT(instance == SignalManagerSingleton::instance());
    }

    void testRegisterHandler() {
        SignalCounter counter;
        CPPUNIT_ASSERT_EQUAL(0ul, counter.getCount());
        SignalManagerSingleton::instance()->registerHandler(SIGUSR1, &counter);
        const unsigned long count = 10;
        for (unsigned long i = 0; i < count; ++i) {
            kill(getpid(), SIGUSR1);
            CPPUNIT_ASSERT_EQUAL(i+1, counter.getCount());
        }
        SignalManagerSingleton::instance()->removeHandler(SIGUSR1);
    }

    void testRemoveHandler() {
        SignalCounter counter;
        CPPUNIT_ASSERT_EQUAL(0ul, counter.getCount());
        SignalManagerSingleton::instance()->registerHandler(SIGUSR1, &counter);
        kill(getpid(), SIGUSR1);
        CPPUNIT_ASSERT_EQUAL(1ul, counter.getCount());
        SignalManagerSingleton::instance()->removeHandler(SIGUSR1);
        kill(getpid(), SIGUSR1);
        CPPUNIT_ASSERT_EQUAL(1ul, counter.getCount());
    }

    void testMultiple() {
        SignalCounter counter1;
        SignalCounter counter2;
        SignalManagerSingleton::instance()->registerHandler(SIGUSR1, &counter1);
        SignalManagerSingleton::instance()->registerHandler(SIGUSR2, &counter2);

        kill(getpid(), SIGUSR1);
        CPPUNIT_ASSERT_EQUAL(1ul, counter1.getCount());
        CPPUNIT_ASSERT_EQUAL(0ul, counter2.getCount());

        kill(getpid(), SIGUSR2);
        CPPUNIT_ASSERT_EQUAL(1ul, counter1.getCount());
        CPPUNIT_ASSERT_EQUAL(1ul, counter2.getCount());

        SignalManagerSingleton::instance()->removeHandler(SIGUSR1);
        SignalManagerSingleton::instance()->removeHandler(SIGUSR2);
    }

  };

} // namespace askap

#endif // #ifndef
