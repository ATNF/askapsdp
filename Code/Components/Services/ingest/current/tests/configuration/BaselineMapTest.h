/// @file 
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

// Support classes
#include <string>

// Classes to test
#include "configuration/BaselineMap.h"

using namespace std;

namespace askap {
namespace cp {
namespace ingest {

class BaselineMapTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(BaselineMapTest);
        CPPUNIT_TEST(testLookup);
        CPPUNIT_TEST(testNoMatchAnt1);
        CPPUNIT_TEST(testNoMatchAnt2);
        CPPUNIT_TEST(testNoMatchPol);
        CPPUNIT_TEST_SUITE_END();

    public:

        void testLookup() {
            LOFAR::ParameterSet params;
            params.add("baselineids","[0,1,4]");
            params.add("0","[0,0,XX]");
            params.add("1","[1,3,XY]");
            params.add("4","[3,1,YY]");
            BaselineMap bm(params);

            CPPUNIT_ASSERT_EQUAL(1, bm.getID(1,3,casa::Stokes::XY));
            CPPUNIT_ASSERT_EQUAL(4, bm.maxID());
            CPPUNIT_ASSERT_EQUAL(size_t(3), bm.size());
            CPPUNIT_ASSERT_EQUAL(0, bm.idToAntenna1(0));
            CPPUNIT_ASSERT_EQUAL(0, bm.idToAntenna2(0));
            CPPUNIT_ASSERT_EQUAL(casa::Stokes::XX, bm.idToStokes(0));
            CPPUNIT_ASSERT_EQUAL(1, bm.idToAntenna1(1));
            CPPUNIT_ASSERT_EQUAL(3, bm.idToAntenna2(1));
            CPPUNIT_ASSERT_EQUAL(casa::Stokes::XY, bm.idToStokes(1));
            CPPUNIT_ASSERT_EQUAL(3, bm.idToAntenna1(4));
            CPPUNIT_ASSERT_EQUAL(1, bm.idToAntenna2(4));
            CPPUNIT_ASSERT_EQUAL(casa::Stokes::YY, bm.idToStokes(4));

            CPPUNIT_ASSERT_EQUAL(0, testNoMatch(3,1,casa::Stokes::XX));
        };

        int32_t testNoMatch(const int32_t ant1, const int32_t ant2, const casa::Stokes::StokesTypes pol) {
            LOFAR::ParameterSet params;
            params.add("baselineids","[0]");
            params.add("0","[3,1,XX]");
            BaselineMap bm(params);

            CPPUNIT_ASSERT_EQUAL(0, bm.getID(3,1,casa::Stokes::XX));
            CPPUNIT_ASSERT_EQUAL(0, bm.maxID());
            CPPUNIT_ASSERT_EQUAL(size_t(1), bm.size());
            return bm.getID(ant1,ant2,pol);
        }
        
        void testNoMatchAnt1() {
            CPPUNIT_ASSERT_EQUAL(-1, testNoMatch(1, 1, casa::Stokes::XX));
        }

        void testNoMatchAnt2() {
            CPPUNIT_ASSERT_EQUAL(-1, testNoMatch(3,2,casa::Stokes::XX));
        }

        void testNoMatchPol() {
            CPPUNIT_ASSERT_EQUAL(-1, testNoMatch(3, 1, casa::Stokes::XY));
            CPPUNIT_ASSERT_EQUAL(-1, testNoMatch(3, 1, casa::Stokes::YY));
            CPPUNIT_ASSERT_EQUAL(-1, testNoMatch(3, 1, casa::Stokes::XY));
        }
};

}   // End namespace ingest
}   // End namespace cp
}   // End namespace askap
