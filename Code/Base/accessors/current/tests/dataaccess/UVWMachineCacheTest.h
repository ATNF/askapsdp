/// @file
/// $brief Unit tests of the uvw machine cache
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
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Max Voronkov <maxim.voronkov@csiro.au>
/// 

#ifndef UVW_MACHINE_CACHE_TEST_H
#define UVW_MACHINE_CACHE_TEST_H

#include <dataaccess/UVWMachineCache.h>

#include <cppunit/extensions/HelperMacros.h>
#include <casa/Quanta/MVDirection.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/UVWMachine.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Quanta.h>
#include <scimath/Mathematics/RigidVector.h>


#include <boost/shared_ptr.hpp>

namespace askap {

namespace accessors {

class UVWMachineCacheTest : public CppUnit::TestFixture {
   CPPUNIT_TEST_SUITE(UVWMachineCacheTest);
   CPPUNIT_TEST(uvwMachineTest);
   CPPUNIT_TEST_EXCEPTION(exceptionTest,AskapError);
   CPPUNIT_TEST(oneElementCacheTest);
   CPPUNIT_TEST(twoElementsCacheTest);
   CPPUNIT_TEST_SUITE_END();
public:
   void setUp() {
      itsMachineCache.reset();
   }
   
   void exceptionTest() {
      itsMachineCache.reset(new UVWMachineCache(0,1e-6));
      testCaching();
   };
   
   /// @brief calculate uvw from first principles
   /// @details
   /// @param[in] uvw a vector to fill
   /// @param[in] baselines a vector with baseline coordinates (global XYZ)
   /// @param[in] dir direction corresponding to the tangent point on the sky
   void calculateUVW(casa::Vector<casa::RigidVector<double, 3> > &uvw,
                     const casa::Vector<casa::RigidVector<double, 3> > &baselines,
                     const casa::MVDirection &dir)
   {
      const size_t size = baselines.nelements();
      uvw.resize(size);
      const double sDec = sin(dir.getLat());
      const double cDec = cos(dir.getLat());
      const double gmst = casa::C::pi; // some random sidereal time
      const double sH0 = sin(gmst - dir.getLong());
      const double cH0 = cos(gmst - dir.getLong());
      for (casa::uInt row = 0; row<size; ++row) {
           uvw[row](0) = sH0 * baselines[row](0) + cH0 * baselines[row](1);
           uvw[row](1) = -sDec * cH0 * baselines[row](0) + sDec * sH0 * baselines[row](1) + cDec * baselines[row](2);
           uvw[row](2) = cDec * cH0 * baselines[row](0) - cDec * sH0 * baselines[row](1) + sDec * baselines[row](2); 
      }      
   }
   
   /// @brief get a quantity from a string
   /// @details
   /// @param[in] str input string
   /// @return value in radians
   double convert(const std::string &str) {
       casa::Quantity q;      
       casa::Quantity::read(q, str);
       return q.getValue(casa::Unit("rad"));   
   }      
   
   /// @brief test uvw machine
   /// @details
   /// @param[in] baselines vector with baseline coordinates in XYZ
   /// @param[in] raOffset offset in RA (degrees)
   /// @param[in] decOffset offset in Dec (degrees)
   /// @param[in] dec declination of the unshifted direction given as string (i.e. "-45.00.00.0")
   /// @return largest absolute difference in baseline coordinates
   double doUVWMachineTest(const casa::Vector<casa::RigidVector<double, 3> > &baselines,
                double raOffset, double decOffset, const std::string &dec) {

      // unshifted direction
      const casa::MVDirection tangent(convert("12h30m00.000"),convert(dec));
      const casa::MDirection dir1(tangent, casa::MDirection::J2000);

      // dir2 is offset from dir1
      casa::MDirection dir2(dir1);
      dir2.shift(raOffset*casa::C::pi/180.,decOffset*casa::C::pi/180.,casa::True);

      // get uvw's from first principles for dir1 and dir2 for the same antenna layout
      casa::Vector<casa::RigidVector<double, 3> > uvw1, uvw2;
      calculateUVW(uvw1, baselines, dir1.getValue());
      calculateUVW(uvw2, baselines, dir2.getValue());
      
      // rotate uvw via UVWMachine to the original unshifted tangent point
      casa::UVWMachine machine(dir2,dir1,false, true);
      for (size_t row=0; row<uvw2.nelements(); ++row) {
           casa::Vector<double> buf = uvw2[row].vector();
           machine.convertUVW(buf);
           uvw2[row] = buf;
      }
      
      // compare with the uvw's obtained for the original unshifted direction
      double maxDiff = -1;
      for (size_t row=0; row<uvw2.nelements(); ++row) {
           const casa::RigidVector<double, 3> uvwDiff = uvw2[row] - uvw1[row];
           for (int dim=0; dim<3; ++dim) {
                const double curDiff = fabs(uvwDiff(dim));
                if (curDiff > maxDiff) {
                    maxDiff = curDiff;
                }                    
           }
      }
      return maxDiff;
   }
   
   void uvwMachineTest() {
      // this is actually a test of the UVWMachine, not of our code
      // intended to be adapted to become a part of casacore

      // array layout as global XYZ
      const size_t nAnt = 6;
      const double layout[nAnt][3] = 
         {{-2.556088250000000e+06, 5.097405500000000e+06, -2.848428250000000e+06},
          {-2.556121750000000e+06, 5.097392000000000e+06, -2.848421500000000e+06},
          {-2.556231500000000e+06, 5.097387500000000e+06, -2.848327500000000e+06},
          {-2.556006250000000e+06, 5.097327500000000e+06, -2.848641500000000e+06},
          {-2.555892500000000e+06, 5.097559500000000e+06, -2.848328750000000e+06},
          {-2.556745500000000e+06, 5.097448000000000e+06, -2.847753750000000e+06}};
          
      casa::Vector<casa::RigidVector<double, 3> > baselines(nAnt*(nAnt-1)/2);
      for (size_t ant1 = 0, row=0; ant1<nAnt; ++ant1) {
           for (size_t ant2 = 0; ant2<ant1; ++ant2,++row) {
                for (int dim=0; dim<3; ++dim) {
                     baselines[row](dim) = layout[ant2][dim] - layout[ant1][dim];
                }
           }
      }
      /*
      std::cout<<" dec -45, offsets 2, 2: "<<doUVWMachineTest(baselines, 2., 2., "-45.00.00.0")<<std::endl;
      std::cout<<" dec -45, offsets 0, 2: "<<doUVWMachineTest(baselines, 0, 2., "-45.00.00.0")<<std::endl;
      std::cout<<" dec 0, offsets 2, 2: "<<doUVWMachineTest(baselines, 2., 2., "00.00.00.0")<<std::endl;
      */
      
      // the tests below impose very loose tolerances, we need to make them more strict when we
      // finally figure out what's going on with the uvw-machine
      CPPUNIT_ASSERT(doUVWMachineTest(baselines, 2., 2., "-45.00.00.0") < 15.);
      CPPUNIT_ASSERT(doUVWMachineTest(baselines, 0, 2., "-45.00.00.0") < 0.2);
      CPPUNIT_ASSERT(doUVWMachineTest(baselines, 2., 2., "00.00.00.0") < 1.5);
   }   
   
      
   void oneElementCacheTest() {
      itsMachineCache.reset(new UVWMachineCache(1,1e-6));
      testCaching();
   };
   
   void twoElementsCacheTest() {
      itsMachineCache.reset(new UVWMachineCache(2,1e-6));
      testCaching();
   }
   
protected:
   void testCaching() const {
      casa::MVDirection dir1(0.123456, -0.123456);
      casa::MVDirection dir2(-0.123456, -0.123456);
      casa::MVDirection dir3(1.123456, -0.2);
      testDirections(dir1,dir2);
      testDirections(dir1,dir2);
      testDirections(dir2,dir1);
      testDirections(dir3,dir1);
      testDirections(dir2,dir3);      
      testDirections(dir2,dir1);
      testDirections(dir3,dir1);         
   }
   
   void testDirections(const casa::MVDirection &dir1, const casa::MVDirection &dir2) const {
      casa::MDirection dir1j2000(dir1, casa::MDirection::J2000);
      casa::MDirection dir2j2000(dir2, casa::MDirection::J2000);
      ASKAPASSERT(itsMachineCache);
      const accessors::UVWMachineCache::machineType &cachedMachine = itsMachineCache->machine(dir1,dir2);
      // create a proper machine by hand
      accessors::UVWMachineCache::machineType machine2(dir2,dir1,false, true);
      compareMachines(cachedMachine, machine2);         
   }
   
   static void compareMachines(const accessors::UVWMachineCache::machineType &m1, const accessors::UVWMachineCache::machineType &m2) {
       casa::Vector<double> uvw(3);
       uvw[0]=1000.0; uvw[1]=-3250.0; uvw[2]=12.5;
       casa::Vector<double> uvwCopy(uvw.copy());
       double delay = 0, delayCopy = 0;
       m1.convertUVW(delay, uvw);
       m2.convertUVW(delayCopy,uvwCopy);
       CPPUNIT_ASSERT(fabs(delay-delayCopy)<1e-6);
       for (size_t dim=0;dim<3;++dim) {
            CPPUNIT_ASSERT(fabs(uvw[dim]-uvwCopy[dim])<1e-6);
       }
   }   
   
private:   
   boost::shared_ptr<UVWMachineCache> itsMachineCache;
};

} // namespace accessors

} // namespace askap

#endif // #ifndef UVW_MACHINE_CACHE_TEST_H
