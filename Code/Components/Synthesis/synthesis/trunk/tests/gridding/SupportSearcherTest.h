/// @file
///
/// Unit test for support searching utilities
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

#include <gridding/SupportSearcher.h>
#include <cppunit/extensions/HelperMacros.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/IPosition.h>
#include <casa/BasicSL/Complex.h>

#include <measurementequation/SynthesisParamsHelper.h>
namespace askap {

namespace synthesis {

class SupportSearcherTest : public CppUnit::TestFixture 
{
   CPPUNIT_TEST_SUITE(SupportSearcherTest);
   CPPUNIT_TEST(testSupportSearch);
   CPPUNIT_TEST(testPeakFind);
   CPPUNIT_TEST_SUITE_END();
public:
   void setUp() {
      itsBuffer.resize(50,50);
      itsXOffset = -3;
      itsYOffset = 7;
      for (int row=0;row<int(itsBuffer.nrow());++row) {
           for (int col=0;col<int(itsBuffer.ncolumn());++col) {
                const double x = double(row-int(itsBuffer.nrow())/2-itsXOffset)/5.;
                const double y = double(col-int(itsBuffer.ncolumn())/2-itsYOffset)/5.;                
                itsBuffer(row,col) = casa::Complex(5.*exp(-(x*x+y*y)/2),0.);
           }
      }
      //SynthesisParamsHelper::saveAsCasaImage("dbg.img", amplitude(itsBuffer));
	  //throw 1;      
   }
   
   void testPeakFind() {
     SupportSearcher ss(1e-2);
     ss.findPeak(itsBuffer);
     CPPUNIT_ASSERT(ss.peakPos().nelements() == 2);
     CPPUNIT_ASSERT(ss.peakPos()(0) == (int(itsBuffer.nrow())/2+itsXOffset));
     CPPUNIT_ASSERT(ss.peakPos()(1) == (int(itsBuffer.ncolumn())/2+itsYOffset));
     CPPUNIT_ASSERT(abs(ss.peakVal()-5.)<1e-7);
   }
   
   void testSupportSearch() {
      SupportSearcher ss(1e-2);
      
   }
   
private:
   /// @brief a buffer for the test array
   casa::Matrix<casa::Complex> itsBuffer;
   
   /// @brief offset of the simulated gaussian in x
   int itsXOffset;
   
   /// @brief offset of the simulated gaussian in y
   int itsYOffset;      
};
    
} // namespace synthesis

} // namespace askap

