/// @file
///
/// Tests of the gaussian noise generator (giving complex numbers, independent real and imaginary part).
///
///
/// @copyright (c) 2007 CSIRO
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

#include <cppunit/extensions/HelperMacros.h>
#include <utils/ComplexGaussianNoise.h>

namespace askap {

namespace scimath {

class ComplexGaussianNoiseTest : public CppUnit::TestFixture 
{
   CPPUNIT_TEST_SUITE(ComplexGaussianNoiseTest);
   CPPUNIT_TEST(testStats);
   CPPUNIT_TEST_SUITE_END();
public:
   void testStats() 
   {
     ComplexGaussianNoise cgn(2.);
     const size_t nTests = 10000;
     casa::Complex mean(0.,0.);
     float reSumSq = 0., imSumSq = 0.;
     for (size_t test = 0; test<nTests; ++test) {
          const casa::Complex val = cgn();
          mean += val;
          reSumSq += casa::square(casa::real(val));
          imSumSq += casa::square(casa::imag(val));
     }
     mean /= float(nTests);
     reSumSq /= float(nTests);
     imSumSq /= float(nTests);
     reSumSq -= casa::square(casa::real(mean));
     imSumSq -= casa::square(casa::imag(mean));
     CPPUNIT_ASSERT_DOUBLES_EQUAL(0., casa::real(mean), 0.02);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(0., casa::imag(mean), 0.02);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(2., reSumSq, 0.01);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(2., imSumSq, 0.01);
   }
}; // ComplexGaussianNoiseTest

} // namespace scimath

} // namespace askap

