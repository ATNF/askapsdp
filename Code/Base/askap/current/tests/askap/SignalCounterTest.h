/// @file SignalCounterTest.h
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

#ifndef ASKAP_SIGNAL_COUNTER_TEST_H
#define ASKAP_SIGNAL_COUNTER_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include "askap/SignalCounter.h"

namespace askap
{
  class SignalCounterTest : public CppUnit::TestFixture {
    
    CPPUNIT_TEST_SUITE(SignalCounterTest);
    CPPUNIT_TEST(testInitialState);
    CPPUNIT_TEST(testCount);
    CPPUNIT_TEST(testReset);
    CPPUNIT_TEST_SUITE_END();
    
  public:
    
    void testInitialState() {
        SignalCounter counter;
        CPPUNIT_ASSERT_EQUAL(0ul, counter.getCount());
    }

    void testCount() {
        SignalCounter counter;
        CPPUNIT_ASSERT_EQUAL(0ul, counter.getCount());
        counter.handleSignal(0);
        CPPUNIT_ASSERT_EQUAL(1ul, counter.getCount());
    }

    void testReset() {
        SignalCounter counter;
        CPPUNIT_ASSERT_EQUAL(0ul, counter.getCount());
        counter.handleSignal(0);
        CPPUNIT_ASSERT_EQUAL(1ul, counter.getCount());
        counter.resetCount();
        CPPUNIT_ASSERT_EQUAL(0ul, counter.getCount());
    }

  };

} // namespace askap

#endif // #ifndef
