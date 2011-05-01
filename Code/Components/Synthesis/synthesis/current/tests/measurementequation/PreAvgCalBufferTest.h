/// @file
/// 
/// @brief Unit tests for PreAvgCalBuffer.
/// @details PreAvgCalBuffer accululates partial sums for a number of
/// visibility groups (indexed by baseline and beam), which are then used
/// in the least square problem avoiding the iteration over the original dataset.
/// This file contains unit tests of this class
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

#ifndef PRE_AVG_CAL_BUFFER_TEST_H
#define PRE_AVG_CAL_BUFFER_TEST_H

#include <dataaccess/DataIteratorStub.h>
#include <cppunit/extensions/HelperMacros.h>
#include <measurementequation/PreAvgCalBuffer.h>
#include <measurementequation/ComponentEquation.h>

#include <askap/AskapError.h>
#include <askap/AskapUtil.h>

#include <boost/shared_ptr.hpp>


namespace askap {

namespace synthesis   {

/// @brief unit tests of PreAvgCalBuffer
class PreAvgCalBufferTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PreAvgCalBufferTest);
  CPPUNIT_TEST(testInitByAccessor);
  CPPUNIT_TEST(testInitExplicit);
  CPPUNIT_TEST(testPolIndex);
  CPPUNIT_TEST(testAccumulate);
  CPPUNIT_TEST(testAccumulateXPol);
  CPPUNIT_TEST_SUITE_END();
      
  private:
     boost::shared_ptr<ComponentEquation> itsME;
     boost::shared_ptr<Params> itsParams;
     accessors::SharedIter<accessors::DataIteratorStub> itsIter;
     
  public:
     void setUp() {
         itsParams.reset(new Params);
         itsParams->add("flux.i.src", 100.);
         itsParams->add("direction.ra.src", 0.5*casa::C::arcsec);
         itsParams->add("direction.dec.src", -0.3*casa::C::arcsec);
         itsParams->add("shape.bmaj.src", 3.0e-3*casa::C::arcsec);
         itsParams->add("shape.bmin.src", 2.0e-3*casa::C::arcsec);
         itsParams->add("shape.bpa.src", -55*casa::C::degree);
         
         itsIter = accessors::SharedIter<accessors::DataIteratorStub>(new accessors::DataIteratorStub(1));
         accessors::DataAccessorStub &da = dynamic_cast<accessors::DataAccessorStub&>(*itsIter);
         ASKAPASSERT(da.itsStokes.nelements() == 1);
         da.itsStokes[0] = casa::Stokes::XX;   
         da.itsNoise.set(1.0);
         
         itsME.reset(new ComponentEquation(*itsParams, itsIter));               
     }
     void testInitByAccessor() {
         PreAvgCalBuffer pacBuf;
         pacBuf.initialise(*itsIter);
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToType());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredNoMatch());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToFlags());         
         CPPUNIT_ASSERT_EQUAL(itsIter->nRow(),pacBuf.nRow());
         CPPUNIT_ASSERT_EQUAL(1u,pacBuf.nChannel());
         CPPUNIT_ASSERT_EQUAL(itsIter->nPol(),pacBuf.nPol());
         CPPUNIT_ASSERT_EQUAL(pacBuf.nRow(),pacBuf.flag().nrow());
         CPPUNIT_ASSERT_EQUAL(pacBuf.nPol(),pacBuf.flag().nplane());
         CPPUNIT_ASSERT_EQUAL(1u,pacBuf.flag().ncolumn());
         CPPUNIT_ASSERT_EQUAL(1u,casa::uInt(pacBuf.stokes().nelements()));
         CPPUNIT_ASSERT_EQUAL(casa::Stokes::XX, pacBuf.stokes()[0]);
                  
         for (casa::uInt row=0; row < pacBuf.nRow(); ++row)  {
              CPPUNIT_ASSERT_EQUAL(itsIter->antenna1()[row],pacBuf.antenna1()[row]);
              CPPUNIT_ASSERT_EQUAL(itsIter->antenna2()[row],pacBuf.antenna2()[row]);
              CPPUNIT_ASSERT_EQUAL(itsIter->feed1()[row],pacBuf.feed1()[row]);
              CPPUNIT_ASSERT_EQUAL(itsIter->feed2()[row],pacBuf.feed2()[row]);
              for (casa::uInt pol=0; pol<pacBuf.nPol(); ++pol) {
                   CPPUNIT_ASSERT_EQUAL(true,pacBuf.flag()(row,0,pol));
              }              
         }
     }
     
     void testInitExplicit() {
         // 20 antennas instead of 30 available, 2 beams instead of 1 available in the stubbed
         // accessor
         PreAvgCalBuffer pacBuf(20,2);
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToType());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredNoMatch());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToFlags());         
         // 20 antennas and 2 beams give 380 rows; 4 polarisation by default
         CPPUNIT_ASSERT_EQUAL(380u,pacBuf.nRow());
         CPPUNIT_ASSERT_EQUAL(1u,pacBuf.nChannel());
         CPPUNIT_ASSERT_EQUAL(4u,pacBuf.nPol());
         CPPUNIT_ASSERT_EQUAL(pacBuf.nRow(),pacBuf.flag().nrow());
         CPPUNIT_ASSERT_EQUAL(pacBuf.nPol(),pacBuf.flag().nplane());
         CPPUNIT_ASSERT_EQUAL(pacBuf.nChannel(),pacBuf.flag().ncolumn());
         CPPUNIT_ASSERT_EQUAL(4u,casa::uInt(pacBuf.stokes().nelements()));
         CPPUNIT_ASSERT(scimath::PolConverter::isLinear(pacBuf.stokes()));
         for (casa::uInt pol = 0; pol<4; ++pol) {
              CPPUNIT_ASSERT_EQUAL(pol,scimath::PolConverter::getIndex(pacBuf.stokes()[pol]));
         }
         
         CPPUNIT_ASSERT(itsME);
         CPPUNIT_ASSERT(itsIter);
         
         // simulate visibilities
         itsME->predict(*itsIter);
         
         pacBuf.accumulate(*itsIter, itsME);
         
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToType());
         // (435 - 190) * 8 = 1960 samples unaccounted for 
         // (accessor has 1 polarisation)
         CPPUNIT_ASSERT_EQUAL(1960u,pacBuf.ignoredNoMatch());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToFlags());         

         for (casa::uInt row=0; row<pacBuf.nRow(); ++row) {
              CPPUNIT_ASSERT_EQUAL(pacBuf.feed1()[row], pacBuf.feed2()[row]);
              for (casa::uInt pol=0; pol<pacBuf.nPol(); ++pol) {
                   if ((pol == 0) && (pacBuf.feed1()[row] == 0)) {
                       CPPUNIT_ASSERT_EQUAL(false, pacBuf.flag()(row,0,pol));
                       CPPUNIT_ASSERT_DOUBLES_EQUAL(double(pacBuf.sumModelAmps()(row,0,pol)),
                                              double(real(pacBuf.sumVisProducts()(row,0,pol))),1e-2);
                       CPPUNIT_ASSERT_DOUBLES_EQUAL(0,double(imag(pacBuf.sumVisProducts()(row,0,pol))),1e-5);
                       // 8 channels and 100 Jy source give sums of 80000 per accessor summed in
                       CPPUNIT_ASSERT_DOUBLES_EQUAL(80000., double(pacBuf.sumModelAmps()(row,0,pol)),1e-2);
                   } else {
                       // nothing should be found in the accessor, so the appropriate samples should be flagged
                       CPPUNIT_ASSERT_EQUAL(true, pacBuf.flag()(row,0,pol));                    
                   }
              }
         }
         
     }
     
     void testPolIndex() {
         // 20 antennas, 1 beam + 4 polarisations by default
         PreAvgCalBuffer pacBuf(20,1);
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToType());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredNoMatch());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToFlags());         
         // 20 antennas and 1 beam give 190 rows; 4 polarisation by default
         CPPUNIT_ASSERT_EQUAL(190u,pacBuf.nRow());
         CPPUNIT_ASSERT_EQUAL(1u,pacBuf.nChannel());
         CPPUNIT_ASSERT_EQUAL(4u,pacBuf.nPol());
         for (casa::uInt pol1 = 0; pol1 < pacBuf.nPol(); ++pol1) {
              for (casa::uInt pol2 = 0; pol2 <= pol1; ++pol2) {
                   const casa::uInt index = pacBuf.polToIndex(pol1,pol2);
                   const std::pair<casa::uInt, casa::uInt> pols = pacBuf.indexToPol(index);
                   CPPUNIT_ASSERT_EQUAL(pol1, pols.first);
                   CPPUNIT_ASSERT_EQUAL(pol2, pols.second);                   
              }
         }
     }
     
     void testResults(const PreAvgCalBuffer &pacBuf, const int run = 1) {
         for (casa::uInt row=0; row<pacBuf.nRow(); ++row) {
              CPPUNIT_ASSERT_EQUAL(pacBuf.feed1()[row], pacBuf.feed2()[row]);
              for (casa::uInt pol=0; pol<pacBuf.nPol(); ++pol) {
                   CPPUNIT_ASSERT_DOUBLES_EQUAL(double(pacBuf.sumModelAmps()(row,0,pol)),double(real(pacBuf.sumVisProducts()(row,0,pol))),1e-2*run);
                   CPPUNIT_ASSERT_DOUBLES_EQUAL(0,double(imag(pacBuf.sumVisProducts()(row,0,pol))),1e-5);
                   // 8 channels and 100 Jy source give sums of 80000 per accessor summed in
                   CPPUNIT_ASSERT_DOUBLES_EQUAL(pol%3 == 0 ? 80000.*run : 0., double(pacBuf.sumModelAmps()(row,0,pol)),1e-2*run);
                   CPPUNIT_ASSERT_EQUAL(false, pacBuf.flag()(row,0,pol));                       
              }
         }
     }
     
     void testAccumulate() {
         PreAvgCalBuffer pacBuf;
         CPPUNIT_ASSERT(itsME);
         CPPUNIT_ASSERT(itsIter);
         
         // simulate visibilities
         itsME->predict(*itsIter);
         
         // buffer should be initialised by the first encountered accessor
         pacBuf.accumulate(*itsIter, itsME);

         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToType());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredNoMatch());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToFlags());
         CPPUNIT_ASSERT_EQUAL(itsIter->nRow(),pacBuf.nRow());
         CPPUNIT_ASSERT_EQUAL(1u,pacBuf.nChannel());
         CPPUNIT_ASSERT_EQUAL(itsIter->nPol(),pacBuf.nPol());
         testResults(pacBuf,1);                  

         // add up another accessor
         pacBuf.accumulate(*itsIter, itsME);         
         testResults(pacBuf,2);                  
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToType());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredNoMatch());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToFlags());         
     }
     
     void testAccumulateXPol() {
         casa::Vector<casa::Stokes::StokesTypes> stokes(4);
         stokes[0] = casa::Stokes::XX;
         stokes[1] = casa::Stokes::XY;
         stokes[2] = casa::Stokes::YX;
         stokes[3] = casa::Stokes::YY;         

         CPPUNIT_ASSERT(itsIter);       
         accessors::DataAccessorStub &da = dynamic_cast<accessors::DataAccessorStub&>(*itsIter);          
         da.itsStokes.assign(stokes.copy());
         da.itsVisibility.resize(da.nRow(), da.nChannel() ,4);
         da.itsVisibility.set(casa::Complex(-10.,15.));
         da.itsNoise.resize(da.nRow(),da.nChannel(),da.nPol());
         da.itsNoise.set(1.);
         da.itsFlag.resize(da.nRow(),da.nChannel(),da.nPol());
         da.itsFlag.set(casa::False);

         CPPUNIT_ASSERT(itsME);
         // simulate visibilities
         itsME->predict(*itsIter);
         PreAvgCalBuffer pacBuf;

         // buffer should be initialised by the first encountered accessor
         pacBuf.accumulate(*itsIter, itsME);
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToType());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredNoMatch());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToFlags());
         CPPUNIT_ASSERT_EQUAL(itsIter->nRow(),pacBuf.nRow());
         CPPUNIT_ASSERT_EQUAL(1u,pacBuf.nChannel());
         CPPUNIT_ASSERT_EQUAL(itsIter->nPol(),pacBuf.nPol());
         testResults(pacBuf,1);                  

         // add up another accessor
         pacBuf.accumulate(*itsIter, itsME);         
         testResults(pacBuf,2);                  
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToType());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredNoMatch());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToFlags());              
     }
};
} // namespace synthesis

} // namespace askap

#endif // #ifndef PRE_AVG_CAL_BUFFER_TEST_H

