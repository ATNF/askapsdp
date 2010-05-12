/// @file AntennaPositionsTest.cc
///
/// @copyright (c) 2010 CSIRO
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
#include "Common/ParameterSet.h"

// Classes to test
#include "ingestutils/AntennaPositions.h"

namespace askap
{
    namespace cp
    {
        class AntennaPositionsTest : public CppUnit::TestFixture
        {
            CPPUNIT_TEST_SUITE(AntennaPositionsTest);
            CPPUNIT_TEST(testSimple);
            CPPUNIT_TEST_SUITE_END();

            public:
            void setUp()
            {
                // Setup a parameter set
                itsParset.add("location", "[+117.471deg, -25.692deg, 192m, WGS84]");
                itsParset.add("names", "[A0, A1, A2, A3, A4, A5]");
                itsParset.add("scale", "1.0");
                itsParset.add("A0", "[-175.233429,  -1673.460938,  0.0000]");
                itsParset.add("A1", "[261.119019,   -796.922119,   0.0000]");
                itsParset.add("A2", "[-29.200520,   -744.432068,   0.0000]");
                itsParset.add("A3", "[-289.355286,  -586.936035,   0.0000]");
                itsParset.add("A4", "[-157.031570,  -815.570068,   0.0000]");
                itsParset.add("A5", "[-521.311646,  -754.674927,   0.0000]");
            };

            void tearDown()
            {
                itsParset.clear();
            }

            void testSimple()
            {
                // Tolerance for double equality
                const double tol = 1.0E-8;

                // Indexing
                const int X = 0;
                const int Y = 1;
                const int Z = 2;

                AntennaPositions antPos(itsParset);
                casa::Matrix<double> antXYZ = antPos.getPositionMatrix();
                CPPUNIT_ASSERT_EQUAL(3U, antXYZ.nrow());
                CPPUNIT_ASSERT_EQUAL(6U, antXYZ.ncolumn());

                // Antenna A0
                CPPUNIT_ASSERT_DOUBLES_EQUAL(-2652616.85460246, antXYZ(X, 0), tol);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(5102312.63799787, antXYZ(Y, 0), tol);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(-2749946.41159169, antXYZ(Z, 0), tol);

                // Antenna A5
                CPPUNIT_ASSERT_DOUBLES_EQUAL(-2652492.54473864, antXYZ(X, 5), tol);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(5102823.76998959, antXYZ(Y, 5), tol);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(-2749117.41882315, antXYZ(Z, 5), tol);
            };

            private:
            LOFAR::ParameterSet itsParset;

        };

    }   // End namespace cp

}   // End namespace askap
