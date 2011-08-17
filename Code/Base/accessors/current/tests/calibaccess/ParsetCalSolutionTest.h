/// @file
///
/// Unit test for the parset-based implementation of the interface to access
/// calibration solutions
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
#include <calibaccess/ParsetCalSolutionSource.h>
#include <calibaccess/ParsetCalSolutionAccessor.h>

#include <boost/shared_ptr.hpp>


namespace askap {

namespace accessors {

class ParsetCalSolutionTest : public CppUnit::TestFixture 
{
   CPPUNIT_TEST_SUITE(ParsetCalSolutionTest);
   CPPUNIT_TEST(testReadWrite);
   CPPUNIT_TEST_SUITE_END();
protected:
   static void createDummyParset(const std::string &fname) {
       ParsetCalSolutionAccessor acc(fname);
       for (casa::uInt ant=0; ant<5; ++ant) {
            for (casa::uInt beam=0; beam<4; ++beam) { 
                 const float tag = float(ant)/100. + float(beam)/1000.;
                 acc.setJonesElement(ant,beam,casa::Stokes::XX,casa::Complex(1.1+tag,0.1));
                 acc.setJonesElement(ant,beam,casa::Stokes::YY,casa::Complex(1.1,-0.1-tag));
                 acc.setJonesElement(ant,beam,casa::Stokes::XY,casa::Complex(0.1+tag,-0.1));
                 acc.setJonesElement(ant,beam,casa::Stokes::YX,casa::Complex(-0.1,0.1+tag));
            }
       }
   }   
public:
   void testReadWrite() {
        const std::string fname = "tmp.testparset";
        createDummyParset(fname);
        
   }
};

} // namespace accessors

} // namespace askap

