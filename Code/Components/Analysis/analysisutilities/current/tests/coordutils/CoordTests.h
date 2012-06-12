/// @file
///
/// XXX Notes on program XXX
///
/// @copyright (c) 2008 CSIRO
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#include <coordUtils/PositionUtilities.h>
#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <math.h>

namespace askap {

    namespace analysisutilities {

        class CoordTest : public CppUnit::TestFixture {
                CPPUNIT_TEST_SUITE(CoordTest);
                CPPUNIT_TEST(testDegToDMS);
                CPPUNIT_TEST(testDMStoDeg);
                CPPUNIT_TEST(circular);
                CPPUNIT_TEST_SUITE_END();

            private:
                double deg1, deg2, deg3, deg4;
                std::string dms1, dms2, dms3, dms4;

            public:

                void setUp() {
                    deg1 = 187.5;
                    deg2 = -45.3908333333;
                    deg3 = 188.73625;
                    deg4 = deg1 + 2.*cos(deg2);

                    dms1 = "12:30:00.00";
                    dms2 = "-45:23:27.00";
                    dms3 = "12:34:56.70";
                    dms4 = "12:31:17.5436";
                }

                void tearDown() {};

                void testDegToDMS() {
                    CPPUNIT_ASSERT(decToDMS(deg1, "RA") == dms1);
                    CPPUNIT_ASSERT(decToDMS(deg2, "DEC") == dms2);
                    CPPUNIT_ASSERT(decToDMS(deg3, "RA") == dms3);
                    CPPUNIT_ASSERT(decToDMS(deg4, "RA", 4) == dms4);
                }

                void testDMStoDeg() {
                    CPPUNIT_ASSERT(fabs(dmsToDec(dms1)*15. - deg1) < 1.e-7);
                    CPPUNIT_ASSERT(fabs(dmsToDec(dms2) - deg2) < 1.e-7);
                    CPPUNIT_ASSERT(fabs(dmsToDec(dms3)*15. - deg3) < 1.e-7);
                    CPPUNIT_ASSERT(fabs(dmsToDec(dms4)*15. - deg4) < 1.5e-6);
                }

                void circular() {
                    CPPUNIT_ASSERT(decToDMS(dmsToDec(dms1)*15., "RA") == dms1);
                    CPPUNIT_ASSERT(decToDMS(dmsToDec(dms2), "DEC") == dms2);
                    CPPUNIT_ASSERT(decToDMS(dmsToDec(dms3)*15., "RA") == dms3);
                    CPPUNIT_ASSERT(decToDMS(dmsToDec(dms4)*15., "RA", 4) == dms4);

                    CPPUNIT_ASSERT(fabs(dmsToDec(decToDMS(deg1, "RA"))*15. - deg1) < 1.e-7);
                    CPPUNIT_ASSERT(fabs(dmsToDec(decToDMS(deg2, "DEC")) - deg2) < 1.e-7);
                    CPPUNIT_ASSERT(fabs(dmsToDec(decToDMS(deg3, "RA"))*15. - deg3) < 1.e-7);
                    CPPUNIT_ASSERT(fabs(dmsToDec(decToDMS(deg4, "RA", 4))*15. - deg4) < 1.5e-6);
                }
        };

    }

}
