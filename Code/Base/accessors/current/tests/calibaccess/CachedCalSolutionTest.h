/// @file
///
/// Unit test for a simple memory cache implementation of the interface to access
/// calibration solutions (essentially scimath::Params with specialised interface)
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
#include <calibaccess/CachedCalSolutionAccessor.h>
#include <calibaccess/JonesIndex.h>

#include <boost/shared_ptr.hpp>


namespace askap {

namespace accessors {

class CachedCalSolutionTest : public CppUnit::TestFixture 
{
   CPPUNIT_TEST_SUITE(CachedCalSolutionTest);
   CPPUNIT_TEST(testReadWrite);
   CPPUNIT_TEST(testPartiallyUndefined);
   CPPUNIT_TEST_SUITE_END();
protected:
   static void createDummyParams(ICalSolutionAccessor &acc) {
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
      
   static void testComplex(const casa::Complex &expected, const casa::Complex &obtained, const float tol = 1e-5) {
      CPPUNIT_ASSERT_DOUBLES_EQUAL(real(expected),real(obtained),tol);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(imag(expected),imag(obtained),tol);      
   }
   
   static void testDummyParams(const ICalSolutionConstAccessor &acc) {
        for (casa::uInt ant=0; ant<5; ++ant) {
            for (casa::uInt beam=0; beam<4; ++beam) { 
                 CPPUNIT_ASSERT(acc.jonesValid(ant,beam,0));
                 const casa::SquareMatrix<casa::Complex, 2> jones = acc.jones(ant,beam,0);
                 const float tag = float(ant)/100. + float(beam)/1000.;
                 testComplex(casa::Complex(1.1+tag,0.1), jones(0,0));
                 testComplex(casa::Complex(1.1,-0.1-tag), jones(1,1));
                 testComplex(casa::Complex(0.1+tag,-0.1) * casa::Complex(1.1+tag,0.1), jones(0,1));
                 testComplex(casa::Complex(-0.1,0.1+tag) * casa::Complex(1.1,-0.1-tag), -jones(1,0));
                                                   
                 const JonesIndex index(ant,beam); 
                 CPPUNIT_ASSERT(index.antenna() == casa::Short(ant));                
                 CPPUNIT_ASSERT(index.beam() == casa::Short(beam));                
                 
                 const casa::SquareMatrix<casa::Complex, 2> jones2 = acc.jones(index,10);
                 testComplex(casa::Complex(1.1+tag,0.1), jones2(0,0));
                 testComplex(casa::Complex(1.1,-0.1-tag), jones2(1,1));
                 testComplex(casa::Complex(0.1+tag,-0.1) * casa::Complex(1.1+tag,0.1), jones2(0,1));
                 testComplex(casa::Complex(-0.1,0.1+tag) * casa::Complex(1.1,-0.1-tag), -jones2(1,0));
                 
                 
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
   
public:
   void testReadWrite() {
        CachedCalSolutionAccessor acc;
        createDummyParams(acc);
        CPPUNIT_ASSERT_EQUAL(80u, acc.cache().size());
        testDummyParams(acc);

        boost::shared_ptr<scimath::Params> params(new scimath::Params);
        CPPUNIT_ASSERT(params);
        CachedCalSolutionAccessor acc2(params);
        createDummyParams(acc2);
        CPPUNIT_ASSERT_EQUAL(80u, acc2.cache().size());
        CPPUNIT_ASSERT_EQUAL(80u, params->size());
        testDummyParams(acc2);        
        // check reference semantics
        std::vector<std::string> parlist = params->names();
        for (std::vector<std::string>::const_iterator ci = parlist.begin(); ci!=parlist.end(); ++ci) {
             CPPUNIT_ASSERT(acc2.cache().has(*ci));
             testComplex(params->complexValue(*ci), acc2.cache().complexValue(*ci));
        }
   }
      
   void testPartiallyUndefined() {
        const JonesIndex index(0u,0u); 
        CachedCalSolutionAccessor acc;
        const JonesJTerm gains(casa::Complex(1.1,0.1), true, casa::Complex(1.05,-0.1), false);
        CPPUNIT_ASSERT_EQUAL(0u, acc.cache().size());
        acc.setGain(index, gains);
        CPPUNIT_ASSERT_EQUAL(1u, acc.cache().size());        
        const JonesDTerm leakages(casa::Complex(0.13,-0.12),false, casa::Complex(-0.14,0.11), true);        
        acc.setLeakage(index, leakages);
        CPPUNIT_ASSERT_EQUAL(2u, acc.cache().size());        
        
        // now read and check
        CPPUNIT_ASSERT_EQUAL(false, acc.jonesValid(index,0));
        const casa::SquareMatrix<casa::Complex, 2> jones = acc.jones(index,0);
        
        testComplex(casa::Complex(1.1,0.1), jones(0,0));
        // undefined gain is one
        testComplex(casa::Complex(1.0,0.), jones(1,1));
        // undefined leakage is zero
        testComplex(casa::Complex(0.,0.), jones(0,1));
        testComplex(casa::Complex(-0.14,0.11), -jones(1,0));     
        
        // remove the parameters manually
        const std::string par1 = CalParamNameHelper::paramName(index.antenna(),index.beam(),casa::Stokes::XX);
        const std::string par2 = CalParamNameHelper::paramName(index.antenna(),index.beam(),casa::Stokes::YX);        
        CPPUNIT_ASSERT(acc.cache().has(par1));
        CPPUNIT_ASSERT(acc.cache().has(par2));
        acc.cache().remove(par1);
        acc.cache().remove(par2);

        // now read again and check
        CPPUNIT_ASSERT_EQUAL(false, acc.jonesValid(index,0));
        const casa::SquareMatrix<casa::Complex, 2> jones2 = acc.jones(index,0);
        testComplex(casa::Complex(1.0,0.), jones2(0,0));
        testComplex(casa::Complex(1.0,0.), jones2(1,1));
        testComplex(casa::Complex(0.,0.), jones2(0,1));
        testComplex(casa::Complex(0.,0.), -jones2(1,0));
   }
 
   /*  
   void testSolutionSource() {
        const std::string fname = "tmp.testparset";
        ParsetCalSolutionSource ss(fname);
        long id = ss.newSolutionID(0.);
        boost::shared_ptr<ICalSolutionAccessor> rwAcc = ss.rwSolution(id);
        CPPUNIT_ASSERT(rwAcc);
        createDummyParset(*rwAcc);
        CPPUNIT_ASSERT_EQUAL(id, ss.mostRecentSolution());
        CPPUNIT_ASSERT_EQUAL(id, ss.solutionID(1e-6));
        boost::shared_ptr<ICalSolutionConstAccessor> roAcc = ss.roSolution(id);
        CPPUNIT_ASSERT(roAcc);
        testDummyParset(*roAcc);
   }
   */
};

} // namespace accessors

} // namespace askap

