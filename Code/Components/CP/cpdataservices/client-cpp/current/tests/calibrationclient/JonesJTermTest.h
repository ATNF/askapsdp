/// @file JonesJTermTest.h
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
#include "casa/aipstype.h"
#include "casa/BasicSL/Complex.h"

// Classes to test
#include "calibrationclient/JonesJTerm.h"

namespace askap {
namespace cp {
namespace caldataservice {

class JonesJTermTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(JonesJTermTest);
        CPPUNIT_TEST(testDefaultConstructor);
        CPPUNIT_TEST(testConstructor);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        };

        void tearDown() {
        }

        void testDefaultConstructor() {
            JonesJTerm jterm;
            CPPUNIT_ASSERT_EQUAL(casa::DComplex(-1.0, -1.0), jterm.g1());
            CPPUNIT_ASSERT_EQUAL(false, jterm.g1IsValid());
            CPPUNIT_ASSERT_EQUAL(casa::DComplex(-1.0, -1.0), jterm.g2());
            CPPUNIT_ASSERT_EQUAL(false, jterm.g2IsValid());
        }

        void testConstructor() {
            casa::DComplex g1(1.0, 1.0);            
            casa::Bool g1Valid = true;
            casa::DComplex g2(1.1, 1.1);
            casa::Bool g2Valid = false;
            JonesJTerm jterm(g1, g1Valid, g2, g2Valid);

            CPPUNIT_ASSERT_EQUAL(g1, jterm.g1());
            CPPUNIT_ASSERT_EQUAL(g1Valid, jterm.g1IsValid());
            CPPUNIT_ASSERT_EQUAL(g2, jterm.g2());
            CPPUNIT_ASSERT_EQUAL(g2Valid, jterm.g2IsValid());
        }

    private:
};

}   // End namespace caldataservice
}   // End namespace cp
}   // End namespace askap
