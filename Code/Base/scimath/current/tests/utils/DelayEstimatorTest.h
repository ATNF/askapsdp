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
#include <casa/BasicSL/Constants.h>
#include <casa/BasicMath/Random.h>


namespace askap {

namespace scimath {

class DelayEstimatorTest : public CppUnit::TestFixture 
{
   CPPUNIT_TEST_SUITE(DelayEstimatorTest);
   CPPUNIT_TEST(testEstimation);
   CPPUNIT_TEST(testZeroDelay);
   CPPUNIT_TEST(testFFTBasedEstimation);
   CPPUNIT_TEST(testFFTBasedEstimation2);
   CPPUNIT_TEST(testQuality);
   CPPUNIT_TEST_SUITE_END();
public:
   void testZeroDelay() {
      DelayEstimator de(1e6);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0., de.quality(), 1e-6);
      casa::Vector<casa::Complex> buf(1024,casa::Complex(0.,0.));
      const double delay = de.getDelay(buf);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0., delay, 1e-6);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1., de.quality(), 1e-6);      
   }
   
   void testEstimation() {
      DelayEstimator de(1.); // resolution 1 Hz
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0., de.quality(), 1e-6);
      casa::Vector<casa::Complex> buf(1024);
      // fill the spectrum with a period of 50 channels -> delay of 1/50. seconds
      fillTestSpectrum(buf);
      const double delay = de.getDelay(buf);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1./50., delay, 1e-6);       
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1., de.quality(), 1e-6);
   }
   
   void testFFTBasedEstimation() {
      DelayEstimator de(1.); // resolution 1 Hz
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0., de.quality(), 1e-6);
      casa::Vector<casa::Complex> buf(1024);
      // fill the spectrum with a period of 50 channels -> delay of 1/50. seconds
      fillTestSpectrum(buf);
      const double delay = de.getDelayWithFFT(buf);
      // uncertainty is half the channel width in the lag domain
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1./50., delay, 0.5/1024.);             
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1., de.quality(), 5e-3);
   }

   void testFFTBasedEstimation2() {
      DelayEstimator de(1e3); // resolution 1 kHz
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0., de.quality(), 1e-6);
      casa::Vector<casa::Complex> buf(2048);
      // fill the spectrum with a period of 50 channels -> delay of 20 microseconds
      fillTestSpectrum(buf);
      const double delay = de.getDelayWithFFT(buf);
      // uncertainty is half the channel width in the lag domain
      CPPUNIT_ASSERT_DOUBLES_EQUAL(20e-6, delay, 5e-4/2048.);             
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1., de.quality(), 1e-3);
   }
   
   void testQuality() {
      // real phase bandpass from Virgo observation
      const casa::uInt nChan = 304;
      const float phases[nChan] = {-103.264, -103.034, -102.63, -102.851, -102.961, -102.191, -102.52, 
                                 -102.01, -101.29, -101.139, -100.68, -100.365, -99.5515, -99.5903, -100.05, 
                                 -99.7936, -100.167, -99.1796, -98.97, -99.3743, -98.5759, -98.587, -98.5989, 
                                 -98.1474, -97.8239, -97.4769, -97.0721, -96.333, -96.5582, -96.6102, -97.1096, 
                                 -97.2434, -96.7724, -97.2108, -96.6499, -96.7286, -96.5803, -96.5001, -96.8294, 
                                 -96.971, -96.4209, -96.3851, -96.2093, -95.2351, -94.7377, -93.9041, -93.9892, 
                                 -94.2381, -94.2574, -94.9519, -94.5411, -95.0331, -95.3779, -95.3937, -95.4616, 
                                 -95.6218, -95.8733, -95.6447, -95.7072, -94.5298, -94.0904, -93.6886, -93.3541, 
                                 -94.7274, -95.0391, -95.5154, -94.645, -95.4938, -95.2256, -94.6884, -94.5322, 
                                 -94.6009, -94.5151, -94.7338, -94.4366, -94.0348, -94.0398, -94.0682, -94.1488, 
                                 -93.9765, -93.7548, -93.5649, -93.7678, -94.0838, -94.6936, -94.7007, -95.6151, 
                                 -95.0707, -95.2954, -95.5525, -95.2733, -95.0846, -94.9403, -94.863, -95.1995, 
                                 -95.1636, -94.6676, -95.4671, -94.8692, -94.7588, -94.8964, -94.7481, -94.55, 
                                 -94.3953, -94.9523, -94.2857, -94.4225, -93.9347, -94.2216, -93.6561, -93.3397, 
                                 -92.6933, -92.5404, -93.3523, -93.2643, -93.1468, -93.616, -93.5286, -93.0868, 
                                 -92.2103, -92.4572, -93.0036, -92.892, -92.1765, -92.3461, -92.2541, -91.9173, 
                                 -92.3209, -91.5185, -92.2164, -92.4093, -91.6728, -91.0641, -92.2493, -91.2058, 
                                 -91.8391, -91.4481, -91.2749, -92.3781, -92.087, -91.254, -91.5159, -91.4462, 
                                 -91.2975, -91.2444, -91.0509, -91.3872, -91.3479, -91.3713, -91.0138, -91.2801, 
                                 -90.914, -91.1648, -90.6965, -90.5413, -90.6567, -90.3581, -90.2144, -89.6266, 
                                 -89.659, -90.1333, -90.3924, -89.7453, -90.4993, -91.0469, -91.5811, -91.0976, 
                                 -91.4563, -91.6797, -91.0643, -91.3111, -90.8793, -90.4677, -90.3383, -90.2123, 
                                 -90.4861, -89.6732, -90.6036, -90.5949, -90.6009, -90.6372, -91.0853, -90.0629, 
                                 -90.5784, -90.6729, -89.5813, -90.7924, -90.6198, -90.3291, -90.8095, -91.4352, 
                                 -90.4398, -91.752, -89.4328, -90.5786, -90.8587, -90.0768, -89.5682, -90.2372, 
                                 -90.0077, -90.1803, -90.1605, -90.0484, -89.8356, -89.5054, -89.5673, -90.4804, 
                                 -89.6556, -90.2881, -90.2296, -91.7227, -90.4707, -91.1836, -90.4872, -89.7727, 
                                 -90.5806, -90.4921, -90.4385, -89.4704, -90.0202, -89.4647, -90.8004, -90.652, 
                                 -90.5807, -91.5295, -90.2374, -90.1895, -91.0172, -90.9875, -91.3975, -91.2703, 
                                 -89.4196, -89.601, -89.4456, -91.2219, -89.8748, -91.096, -89.1141, -90.3177, 
                                 -89.7179, -89.0015, -89.6335, -90.2238, -88.9883, -90.4067, -89.3074, -90.6182, 
                                 -89.5539, -90.9037, -90.6836, -90.7206, -90.1027, -90.6621, -91.1437, -90.9358, 
                                 -93.1463, -92.4603, -93.6215, -94.7018, -94.0678, -93.2048, -94.0464, -93.8582, 
                                 -94.239, -94.2523, -97.7813, -95.5506, -95.5722, -97.0328, -97.9777, -96.5939, 
                                 -97.0789, -96.7528, -96.2026, -98.887, -98.1846, -100.091, -97.8369, -98.8785, 
                                 -99.8199, -98.9538, -100.015, -100.522, -101.252, -102.447, -103.244, -101.148, 
                                 -103.056, -104.98, -104.336, -106.17, -105.576, -107.557, -107.217, -109.346, 
                                 -110.007, -110.45, -111.59, -112.429, -114.32, -115.441, -114.045, -115.565, -116.393};
      casa::Vector<casa::Complex> buf(nChan);
      for (casa::uInt ch = 0; ch<nChan; ++ch) {
           buf[ch] = casa::polar(1., phases[ch] * casa::C::pi / 180.);
      }
      DelayEstimator de(-1e6); // resolution 1 MHz
      double delay = de.getDelay(buf);
      // uncertainty is half the channel width in the lag domain
      CPPUNIT_ASSERT(fabs(delay) < 1e-11);             
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1., de.quality(), 6e-3);
      // zero signal to noise ratio -  random phase with the fixed seed to ensure deterministic result
      casa::MLCG generator(0,10);
      casa::Uniform phaseGen(&generator,0., casa::C::_2pi);
      for (casa::uInt ch = 0; ch<nChan; ++ch) {
           buf[ch] = casa::polar(1., phaseGen()); 
      }
      delay = de.getDelay(buf);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0., de.quality(), 0.1);      
   }
   
private:
   // generate the test spectrum with the phase wrap period of 50 channels
   static void fillTestSpectrum(casa::Vector<casa::Complex> &buf) {
      for (casa::uInt ch=0; ch<buf.nelements(); ++ch) {
           const float phase = 2.*casa::C::pi*float(ch)/50.;
           const casa::Complex phasor(cos(phase),sin(phase));
           buf[ch] = phasor;
      }
   }   
};

} // namespace scimath

} // namespace askap

