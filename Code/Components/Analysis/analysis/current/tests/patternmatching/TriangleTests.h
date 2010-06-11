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
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#include <patternmatching/GrothTriangles.h>
#include <cppunit/extensions/HelperMacros.h>

namespace askap {
    namespace analysis {

        namespace matching {

            class TriangleTest : public CppUnit::TestFixture {
                    CPPUNIT_TEST_SUITE(TriangleTest);
                    CPPUNIT_TEST(testPerimeter);
                    CPPUNIT_TEST(testOrientation);
                    CPPUNIT_TEST(testMatch);
                    CPPUNIT_TEST_SUITE_END();

                private:
                    Triangle *t1, *t2, *t3, *t4;

                public:

                    void setUp() {
                        t1 = new Triangle(4., 2., 5., 9., 1., 4.);
                        t2 = new Triangle(8., 7., 14., 4., 12., 2.);
                        t3 = new Triangle(8., 14., 14., 17., 12., 19.);
                        t4 = new Triangle(1., 22., 4., 24., 5., 17.);
                    }

                    void tearDown() {
                        delete t1;
                        delete t2;
                        delete t3;
                        delete t4;
                    }

                    void testPerimeter() {
                        CPPUNIT_ASSERT((pow(10, t1->perimeter()) - 17.079) < 0.001);
                        CPPUNIT_ASSERT((pow(10, t2->perimeter()) - 15.940) < 0.001);
                        CPPUNIT_ASSERT((pow(10, t3->perimeter()) - 15.940) < 0.001);
                        CPPUNIT_ASSERT((pow(10, t4->perimeter()) - 17.079) < 0.001);
                    }

                    void testOrientation() {
                        CPPUNIT_ASSERT(t1->isClockwise());
                        CPPUNIT_ASSERT(t2->isClockwise());
                        CPPUNIT_ASSERT(!t3->isClockwise());
                        CPPUNIT_ASSERT(!t4->isClockwise());
                    }

                    void testMatch() {
                        CPPUNIT_ASSERT(t1->isMatch(*t4));
                        CPPUNIT_ASSERT(t3->isMatch(*t2));
                        CPPUNIT_ASSERT(!t1->isMatch(*t2));
                    }

            };


        }
    }
}
