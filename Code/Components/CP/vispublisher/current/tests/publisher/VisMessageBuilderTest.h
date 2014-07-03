/// @file VisMessageBuilderTest.h
///
/// @copyright (c) 2014 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

#ifndef ASKAP_CP_VISMESSAGEBUILDERTEST_H
#define ASKAP_CP_VISMESSAGEBUILDERTEST_H

// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

// Support classes
#include <stdint.h>
#include <complex>
#include <vector>
#include "publisher/InputMessage.h"
#include "publisher/VisOutputMessage.h"
#include "publisher/VisElement.h"
#include "TestHelperFunctions.h"

// Classes to test
#include "publisher/VisMessageBuilder.h"

using namespace std;

namespace askap {
namespace cp {
namespace vispublisher {

class VisMessageBuilderTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(VisMessageBuilderTest);
        CPPUNIT_TEST(testBuild);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
            itsInMsg = TestHelperFunctions::createInputMessage();
            N_BEAM = TestHelperFunctions::N_BEAM;
            N_CHAN = TestHelperFunctions::N_CHAN;
            N_POL = TestHelperFunctions::N_POL;
            N_BASELINE = TestHelperFunctions::N_BASELINE;
        }

        void tearDown() {
        }

        void testBuild() {
            const uint32_t chanBegin = 0;
            const uint32_t chanEnd = N_CHAN - 1;

            VisOutputMessage out = VisMessageBuilder::build(itsInMsg,
                    chanBegin, chanEnd);

            CPPUNIT_ASSERT_EQUAL(itsInMsg.timestamp(), out.timestamp());
            CPPUNIT_ASSERT_EQUAL(chanBegin, out.chanBegin());
            CPPUNIT_ASSERT_EQUAL(chanEnd, out.chanEnd());

            std::vector<VisElement>& data = out.data();
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(N_BASELINE * N_BEAM * N_POL),
                    data.size());
        }

    private:

        InputMessage itsInMsg;
        uint32_t N_BEAM;
        uint32_t N_CHAN;
        uint32_t N_POL;
        uint32_t N_BASELINE;
};

}   // End namespace vispublisher
}   // End namespace cp
}   // End namespace askap

#endif
