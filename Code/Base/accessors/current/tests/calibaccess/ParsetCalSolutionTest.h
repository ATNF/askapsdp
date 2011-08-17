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

#include <casa/aipstype.h>
#include <cppunit/extensions/HelperMacros.h>
#include <calibaccess/ParsetCalSolutionSource.h>
#include <calibaccess/ParsetCalSolutionAccessor.h>
#include <calibaccess/JonesIndex.h>

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
   
   void testComplex(const casa::Complex &expected, const casa::Complex &obtained, const float tol = 1e-5) {
      CPPUNIT_ASSERT_DOUBLES_EQUAL(real(expected),real(obtained),tol);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(imag(expected),imag(obtained),tol);      
   }
public:
   void testReadWrite() {
        const std::string fname = "tmp.testparset";
        createDummyParset(fname);
        ParsetCalSolutionAccessor acc(fname);
        for (casa::uInt ant=0; ant<5; ++ant) {
            for (casa::uInt beam=0; beam<4; ++beam) { 
                 CPPUNIT_ASSERT(acc.jonesValid(ant,beam,0));
                 const casa::SquareMatrix<casa::Complex, 2> jones = acc.jones(ant,beam,0);
                 const float tag = float(ant)/100. + float(beam)/1000.;
                 testComplex(casa::Complex(1.1+tag,0.1), jones(0,0));
                 testComplex(casa::Complex(1.1,-0.1-tag), jones(1,1));
                 testComplex(casa::Complex(0.1+tag,-0.1), jones(0,1));
                 testComplex(casa::Complex(-0.1,0.1+tag), jones(1,0));
                 
                 const JonesIndex index(ant,beam); 
                 CPPUNIT_ASSERT(index.antenna() == casa::Short(ant));                
                 CPPUNIT_ASSERT(index.beam() == casa::Short(beam));                
                 const JonesJTerm jTerm = acc.gain(index);
                 CPPUNIT_ASSERT(jTerm.g1IsValid() && jTerm.g2IsValid());
                 testComplex(casa::Complex(1.1+tag,0.1), jTerm.g1());
                 testComplex(casa::Complex(1.1,-0.1-tag), jTerm.g2());
                 
                 const JonesDTerm dTerm = acc.leakage(index);
                 CPPUNIT_ASSERT(dTerm.d12IsValid() && dTerm.d21IsValid());
                 testComplex(casa::Complex(0.1+tag,-0.1), dTerm.d12());
                 testComplex(casa::Complex(-0.1,0.1+tag), dTerm.d21()); 
                 
                 for (casa::uInt chan=0; chan<20; ++chan) {
                      const JonesJTerm bpTerm = acc.bandpass(index, chan);
                      CPPUNIT_ASSERT(bpTerm.g1IsValid() && bpTerm.g2IsValid());
                      testComplex(casa::Complex(1.,0.), bpTerm.g1());
                      testComplex(casa::Complex(1.,0.), bpTerm.g2());
                 }
            }
        }       
        
   }
};

} // namespace accessors

} // namespace askap

