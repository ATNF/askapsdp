/// @file
///
/// Unit test for CalParamNameHelper class (naming convension for
/// the calibration parameters)
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

// own includes
#include <calibaccess/JonesIndex.h>
#include <calibaccess/CalParamNameHelper.h>

// std includes
#include <string>
#include <utility>

namespace askap {

namespace accessors {

class CalParamNameHelperTest : public CppUnit::TestFixture 
{
   CPPUNIT_TEST_SUITE(CalParamNameHelperTest);
   CPPUNIT_TEST(testToString);
   CPPUNIT_TEST(testFromString);
   CPPUNIT_TEST_EXCEPTION(testFromStringException1, AskapError);
   CPPUNIT_TEST_EXCEPTION(testFromStringException2, AskapError);
   CPPUNIT_TEST_EXCEPTION(testFromStringException3, AskapError);
   CPPUNIT_TEST_EXCEPTION(testFromStringException4, AskapError);
   CPPUNIT_TEST_SUITE_END();
public:
   void testToString() {
      CPPUNIT_ASSERT_EQUAL(std::string("gain.g11.21.5"),CalParamNameHelper::paramName(JonesIndex(21u,5u),casa::Stokes::XX));
      CPPUNIT_ASSERT_EQUAL(std::string("gain.g22.11.11"),CalParamNameHelper::paramName(JonesIndex(11u,11u),casa::Stokes::YY));
      CPPUNIT_ASSERT_EQUAL(std::string("leakage.d12.10.1"),CalParamNameHelper::paramName(JonesIndex(10u,1u),casa::Stokes::XY));
      CPPUNIT_ASSERT_EQUAL(std::string("leakage.d21.15.10"),CalParamNameHelper::paramName(JonesIndex(15u,10u),casa::Stokes::YX));      
   }
   
   void doFromStringChecks(const casa::uInt ant, const casa::uInt beam, const casa::Stokes::StokesTypes pol) {
      const JonesIndex index(ant,beam);
      const std::string name = CalParamNameHelper::paramName(index,pol);
      const std::pair<JonesIndex, casa::Stokes::StokesTypes> res = CalParamNameHelper::parseParam(name);
      CPPUNIT_ASSERT((res.first.antenna() >=0) && (res.first.antenna()<256)); 
      CPPUNIT_ASSERT((res.first.beam() >=0) && (res.first.beam()<256)); 
      CPPUNIT_ASSERT_EQUAL(ant, casa::uInt(res.first.antenna()));
      CPPUNIT_ASSERT_EQUAL(beam, casa::uInt(res.first.beam()));
      CPPUNIT_ASSERT(index == res.first);
      CPPUNIT_ASSERT(pol == res.second);                                           
   }
   
   void testFromString() {
      for (casa::uInt ant=0; ant<36; ++ant) {
           for (casa::uInt beam=0; beam<30; ++beam) {
                doFromStringChecks(ant,beam,casa::Stokes::XX);
                doFromStringChecks(ant,beam,casa::Stokes::XY);
                doFromStringChecks(ant,beam,casa::Stokes::YX);
                doFromStringChecks(ant,beam,casa::Stokes::YY);                
           }
      }
   }
   
   void testFromStringException1() {
        CalParamNameHelper::parseParam("something.g11.3.4");
   }

   void testFromStringException2() {
        CalParamNameHelper::parseParam("leakage.junk.3.4");
   }

   void testFromStringException3() {
        CalParamNameHelper::parseParam("leakage.d21.3");
   }

   void testFromStringException4() {
        CalParamNameHelper::parseParam("gain.g11.3.xx");
   }
   
}; // class CalParamNameHelperTest

} // namespace accessors

} // namespace askap

