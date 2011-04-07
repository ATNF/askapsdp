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
  CPPUNIT_TEST(testAccumulate);
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
         
         itsME.reset(new ComponentEquation(*itsParams, itsIter));               
     }
     void testInitByAccessor() {
         PreAvgCalBuffer pacBuf;
         pacBuf.initialise(*itsIter);
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToType());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredNoMatch());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToFlags());         
     }
     
     void testAccumulate() {
         PreAvgCalBuffer pacBuf;
         
         // buffer should be initialised by the first encountered accessor
         pacBuf.accumulate(*itsIter, itsME);

         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToType());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredNoMatch());
         CPPUNIT_ASSERT_EQUAL(0u,pacBuf.ignoredDueToFlags());                  
     }
};
} // namespace synthesis

} // namespace askap

#endif // #ifndef PRE_AVG_CAL_BUFFER_TEST_H

