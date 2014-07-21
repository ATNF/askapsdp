/// @file RandomRealTest.cc
///
/// @copyright (c) 2013 CSIRO
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

// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

// Support classes
#include "askap/AskapError.h"

// Classes to test
#include "simplayback/RandomReal.h"

namespace askap {
namespace cp {

class RandomRealTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(RandomRealTest);
        CPPUNIT_TEST(testGen);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        };

        void tearDown() {
        }

        void testGen() {
            const double lower = 0.0;
            const double upper = 1.0;
            RandomReal<double> rrd(lower, upper);

            const int nTrials = 1000000;
            for (int i = 0; i < nTrials; ++i) {
                const double n = rrd.gen();
                CPPUNIT_ASSERT(n >= lower);
                CPPUNIT_ASSERT(n <= upper);
            }
        }
};

}   // End namespace cp
}   // End namespace askap
