/// @file VOTableTest.cc
///
/// @copyright (c) 2012 CSIRO
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
#include <iostream>

// Classes to test
#include "votable/VOTable.h"

using namespace std;

namespace askap {
namespace accessors {

class VOTableTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(VOTableTest);
        CPPUNIT_TEST(testAll);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        };

        void tearDown() {
        }

        void testAll() {
            VOTable vot1;
            vot1.setDescription("Test Description");

            // Convert to XML
            std::stringstream ss;
            vot1.toXML(ss);

            // Convert XML back to VOTable
            VOTable vot2 = VOTable::fromXML(ss);

            // Verify result
            CPPUNIT_ASSERT(vot2.getDescription().compare(vot1.getDescription()) == 0);
        }
};

}
}
