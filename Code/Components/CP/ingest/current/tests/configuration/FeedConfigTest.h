/// @file FeedConfigTest.cc
///
/// @copyright (c) 2011 CSIRO
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
#include "casa/aips.h"
#include "casa/BasicSL.h"
#include "casa/Quanta.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Vector.h"
#include "boost/scoped_ptr.hpp"

// Classes to test
#include "configuration/FeedConfig.h"

using namespace std;

namespace askap {
namespace cp {
namespace ingest {

class FeedConfigTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(FeedConfigTest);
        CPPUNIT_TEST(testAll);
        CPPUNIT_TEST(testExceptions);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        };

        void tearDown() {
        }

        void testAll() {
            casa::Double dblTolerance = 1e-15;

            // Create an instance to test
            const casa::Double nFeeds = 3;
            casa::Matrix<casa::Quantity> offsets(nFeeds, 2);
            casa::Vector<casa::String> pols(nFeeds);
            for (casa::Int i = 0; i < nFeeds; ++i) {
                pols[i] = "XX YY";
                offsets(i, 0) = casa::Quantity((1.0 * i), "deg"); // X
                offsets(i, 1) = casa::Quantity((2.0 * i), "deg"); // Y
            }
            FeedConfig instance(offsets, pols);

            // Test instance
            for (casa::Int i = 0; i < nFeeds; ++i) {
                CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0 * i, instance.offsetX(i).getValue("deg"), dblTolerance);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0 * i, instance.offsetY(i).getValue("deg"), dblTolerance);
                CPPUNIT_ASSERT(casa::String("XX YY") == instance.pol(i));
            }

        }

        void testExceptions() {
            const casa::Double nFeeds = 3;
            {
                casa::Matrix<casa::Quantity> offsets(nFeeds, 2);
                casa::Vector<casa::String> pols(nFeeds+1);
                CPPUNIT_ASSERT_THROW(FeedConfig(offsets, pols), askap::AskapError);
            }

            {
                casa::Matrix<casa::Quantity> offsets(nFeeds, 1);
                casa::Vector<casa::String> pols(nFeeds);
                CPPUNIT_ASSERT_THROW(FeedConfig(offsets, pols), askap::AskapError);
            }

            {
                casa::Matrix<casa::Quantity> offsets(0, 2);
                casa::Vector<casa::String> pols(0);
                CPPUNIT_ASSERT_THROW(FeedConfig(offsets, pols), askap::AskapError);
            }
        }
};

}   // End namespace ingest
}   // End namespace cp
}   // End namespace askap
