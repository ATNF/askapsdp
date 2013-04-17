/// @file AntennaTest.cc
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
#include "casa/Quanta.h"
#include "casa/BasicSL.h"
#include "casa/Arrays/Vector.h"
#include "configuration/FeedConfig.h"

// Classes to test
#include "configuration/Antenna.h"

using namespace std;

namespace askap {
namespace cp {
namespace ingest {

class AntennaTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(AntennaTest);
        CPPUNIT_TEST(testAll);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        };

        void tearDown() {
        }

        void testAll() {
            casa::Double dblTolerance = 1e-15;

            // Setup objects to create the instance with
            const casa::Int nFeeds = 3;
            casa::Matrix<casa::Quantity> offsets(nFeeds, 2);
            casa::Vector<casa::String> pols(nFeeds);
            for (casa::Int i = 0; i < nFeeds; ++i) {
                pols[i] = "XX YY";
                offsets(i, 0) = casa::Quantity((1.0 * i), "deg"); // X
                offsets(i, 1) = casa::Quantity((2.0 * i), "deg"); // Y
            }
            FeedConfig feedConfig(offsets, pols);

            const casa::String name = "askap01";
            const casa::String mount = "equatorial";
            casa::Vector<casa::Double> position(3);
            position(0) = 1;
            position(1) = 2;
            position(2) = 3;
            const casa::Quantity diameter(12, "m");

            // Create instance
            Antenna instance(name, mount, position, diameter, feedConfig);

            // Test instance
            CPPUNIT_ASSERT(name == instance.name());
            CPPUNIT_ASSERT(mount == instance.mount());
            CPPUNIT_ASSERT(position.nelements() == instance.position().nelements());
            for (size_t i = 0; i < position.nelements(); ++i) {
                CPPUNIT_ASSERT_DOUBLES_EQUAL(position(i), instance.position()(i), dblTolerance);
            }
            CPPUNIT_ASSERT(diameter == instance.diameter());

            // Check exceptional inputs
            const casa::Quantity badDiameter(12, "rad");
            CPPUNIT_ASSERT_THROW(Antenna(name, mount, position, badDiameter, feedConfig),
                    askap::AskapError);
            casa::Vector<casa::Double> badPosition(2);
            CPPUNIT_ASSERT_THROW(Antenna(name, mount, badPosition, diameter, feedConfig),
                    askap::AskapError);
        }
};

}   // End namespace ingest
}   // End namespace cp
}   // End namespace askap
