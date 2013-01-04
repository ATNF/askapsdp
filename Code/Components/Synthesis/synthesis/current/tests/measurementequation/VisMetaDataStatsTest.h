/// @file
/// 
/// @brief Unit tests for VisMetaDataStats class
/// @details VisMetaDataStats accumulates certain statistics of the visibility data.
/// It is used to provide advise capability for the parameters used in imager and
/// calibrator.
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

#ifndef VIS_META_DATA_STATS_ME_TEST_H
#define VIS_META_DATA_STATS_ME_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <measurementequation/VisMetaDataStats.h>
#include <dataaccess/DataAccessorStub.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVDirection.h>


namespace askap
{
  namespace synthesis
  {
    class VisMetaDataStatsTest : public CppUnit::TestFixture
    {
      CPPUNIT_TEST_SUITE(VisMetaDataStatsTest);
      CPPUNIT_TEST(testProcess);
      CPPUNIT_TEST_SUITE_END();
    public:
      void testProcess() {
         accessors::DataAccessorStub acc(true);
         VisMetaDataStats stats;
         CPPUNIT_ASSERT_EQUAL(0ul, stats.nVis());
         stats.process(acc);
         CPPUNIT_ASSERT_DOUBLES_EQUAL(1.4e9,stats.maxFreq(),1.);
         CPPUNIT_ASSERT_DOUBLES_EQUAL(1.260e9,stats.minFreq(),1.);

         // note, we didn't independently verify the following uvw 
         // values, but the magnitude make sense for the stubbed layout         
         CPPUNIT_ASSERT_DOUBLES_EQUAL(4115.62,stats.maxU(),1.);
         CPPUNIT_ASSERT_DOUBLES_EQUAL(3296.23,stats.maxV(),1.);
         CPPUNIT_ASSERT_DOUBLES_EQUAL(6387.41,stats.maxW(),1.);
         bool noResidualW = false;
         try {
             stats.maxResidualW(); // this should fail!
         } 
         catch (const askap::AskapError &) {
             noResidualW = true;
         } 
         CPPUNIT_ASSERT(noResidualW);
         //
         CPPUNIT_ASSERT_EQUAL(30u, stats.nAntennas());
         CPPUNIT_ASSERT_EQUAL(1u, stats.nBeams());
         CPPUNIT_ASSERT_EQUAL(3480ul, stats.nVis());
         CPPUNIT_ASSERT_DOUBLES_EQUAL(0.,stats.maxOffsets().first,1e-6);
         CPPUNIT_ASSERT_DOUBLES_EQUAL(0.,stats.maxOffsets().second,1e-6);
         const casa::MVDirection expectedDir(casa::Quantity(0, "deg"), casa::Quantity(0, "deg"));
         CPPUNIT_ASSERT_DOUBLES_EQUAL(0., expectedDir.separation(stats.centre()), 1e-6);
      } 
      
    };
  
  } // namespace synthesis

} // namespace askap

#endif // #ifndef VIS_META_DATA_STATS_ME_TEST_H

