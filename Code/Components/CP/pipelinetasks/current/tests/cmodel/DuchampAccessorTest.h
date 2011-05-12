/// @file DuchampAccessorTest.cc
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
#include <string>
#include <sstream>
#include "casa/Quanta/Quantum.h"
#include "components/ComponentModels/ComponentList.h"

// Classes to test
#include "cmodel/DuchampAccessor.h"

using namespace casa;

namespace askap {
namespace cp {
namespace pipelinetasks {

class DuchampAccessorTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(DuchampAccessorTest);
        CPPUNIT_TEST(testConeSearchFluxCutoff);
        CPPUNIT_TEST(testConeSearchRadiusCutoff);
        CPPUNIT_TEST(testConeSearchWraparoundRA);
        CPPUNIT_TEST(testConeSearchWraparoundPole);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        };

        void tearDown() {
        }

        void testConeSearchFluxCutoff() {
            std::stringstream ss;
            ss << "# component    galaxy structure right_ascension declination position_angle major_axis minor_axis   i_151   i_610  i_1400  i_4860 i_18000" << std::endl;
            ss << "   12205907  12205907         1      187.500000  -45.000000            0.0        0.0        0.0 -5.0000 -5.0000 -6.0000 -5.0000 -5.0000" << std::endl;
            ss << "    8262173   8262173         1      187.500000  -45.000000            0.0        0.0        0.0 -5.0000 -5.0000 -8.0000 -5.0000 -5.0000" << std::endl;

            casa::Quantity fluxLimit(1, "uJy");
            casa::Quantity radius(5.0, "deg");
            casa::Quantity ra(187.5, "deg");
            casa::Quantity dec(-45.0, "deg");
            DuchampAccessor acc(ss);
            ComponentList list = acc.coneSearch(ra, dec, radius, fluxLimit);
            CPPUNIT_ASSERT_EQUAL(1u, list.nelements());
        }

        void testConeSearchRadiusCutoff() {
            std::stringstream ss;
            ss << "# component    galaxy structure right_ascension declination position_angle major_axis minor_axis   i_151   i_610  i_1400  i_4860 i_18000" << std::endl;
            ss << "   12205907  12205907         1      187.500000  -45.000000            0.0        0.0        0.0 -5.0000 -5.0000 -6.0000 -5.0000 -5.0000" << std::endl;
            ss << "    8262173   8262173         1      150.500000  -45.000000            0.0        0.0        0.0 -5.0000 -5.0000 -6.0000 -5.0000 -5.0000" << std::endl;

            casa::Quantity fluxLimit(1, "uJy");
            casa::Quantity radius(5.0, "deg");
            casa::Quantity ra(187.5, "deg");
            casa::Quantity dec(-45.0, "deg");
            DuchampAccessor acc(ss);
            ComponentList list = acc.coneSearch(ra, dec, radius, fluxLimit);
            CPPUNIT_ASSERT_EQUAL(1u, list.nelements());
        }

        void testConeSearchWraparoundRA() {
            std::stringstream ss;
            ss << "# component    galaxy structure right_ascension declination position_angle major_axis minor_axis   i_151   i_610  i_1400  i_4860 i_18000" << std::endl;
            ss << "   12205907  12205907         1      001.000000  -45.000000            0.0        0.0        0.0 -5.0000 -5.0000 -6.0000 -5.0000 -5.0000" << std::endl;

            casa::Quantity fluxLimit(1, "uJy");
            casa::Quantity radius(2.0, "deg");
            casa::Quantity ra(359.5, "deg");
            casa::Quantity dec(-45.0, "deg");
            DuchampAccessor acc(ss);
            ComponentList list = acc.coneSearch(ra, dec, radius, fluxLimit);
            CPPUNIT_ASSERT_EQUAL(1u, list.nelements());
        }

        void testConeSearchWraparoundPole() {
            std::stringstream ss;
            ss << "# component    galaxy structure right_ascension declination position_angle major_axis minor_axis   i_151   i_610  i_1400  i_4860 i_18000" << std::endl;
            ss << "   12205907  12205907         1      187.500000  -89.900000            0.0        0.0        0.0 -5.0000 -5.0000 -6.0000 -5.0000 -5.0000" << std::endl;

            casa::Quantity fluxLimit(1, "uJy");
            casa::Quantity radius(2.0, "deg");
            casa::Quantity ra(7.5, "deg");
            casa::Quantity dec(-89.5, "deg");
            DuchampAccessor acc(ss);
            ComponentList list = acc.coneSearch(ra, dec, radius, fluxLimit);
            CPPUNIT_ASSERT_EQUAL(1u, list.nelements());
        }

};

}   // End namespace pipelinetasks
}   // End namespace cp
}   // End namespace askap
