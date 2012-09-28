/// @file XercescStringTest.cc
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
#include "xercesc/util/XMLString.hpp"

// Classes to test
#include "votable/XercescString.h"
#include "xercesc/util/PlatformUtils.hpp"

using namespace std;

namespace askap {
namespace accessors {

class XercescStringTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(XercescStringTest);
        CPPUNIT_TEST(testAll);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
            xercesc::XMLPlatformUtils::Initialize();
        };

        void tearDown() {
            xercesc::XMLPlatformUtils::Terminate();
        }

        void testAll() {
            const std::string testStr("my test string");

            XercescString xs1(testStr.c_str());
            CPPUNIT_ASSERT(testStr.compare(xs1) == 0);
            const XMLCh* xc1 = xs1;
            CPPUNIT_ASSERT(xc1 != 0);

            XercescString xs2(testStr);
            CPPUNIT_ASSERT(testStr.compare(xs2) == 0);
        }
};

}
}
