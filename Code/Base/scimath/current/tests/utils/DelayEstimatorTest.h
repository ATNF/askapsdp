/// @file
///
/// Tests of the delay estimator
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

#include <casa/Arrays/Vector.h>
#include <utils/DelayEstimator.h>
#include <casa/BasicSL/Complex.h>

namespace askap {

namespace scimath {

class DelayEstimatorTest : public CppUnit::TestFixture 
{
   CPPUNIT_TEST_SUITE(DelayEstimatorTest);
   CPPUNIT_TEST(testEstimation);
   CPPUNIT_TEST(testZeroDelay);
   CPPUNIT_TEST_SUITE_END();
public:
   void testZeroDelay() {
      DelayEstimator de(1e6);
      casa::Vector<casa::Complex> buf(1024,casa::Complex(0.,0.));
      const double delay = de.getDelay(buf);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0., delay, 1e-6); 
   }
   
   void testEstimation() {
      DelayEstimator de(1.); // resolution 1 Hz
      casa::Vector<casa::Complex> buf(1024,casa::Complex(0.,0.));
      // fill the spectrum with a period of 50 channels -> delay of 1/50. seconds
      for (casa::uInt ch=0; ch<buf.nelements(); ++ch) {
           const float phase = 2.*casa::C::pi*float(ch)/50.;
           const casa::Complex phasor(cos(phase),sin(phase));
           buf[ch] = phasor;
      }
      const double delay = de.getDelay(buf);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1./50., delay, 1e-6);       
   }
};

} // namespace scimath

} // namespace askap

