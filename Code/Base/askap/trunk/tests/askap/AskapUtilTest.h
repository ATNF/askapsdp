/// @file
/// 
/// @brief This file contains tests for macros defined in AskapError.
/// @details The tests cover the ASKAPASSERT and ASKAPCHECK macros
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
/// @author Max Voronkov <Maxim.Voronkov@csiro.au>

#ifndef ASKAP_UTIL_TEST_H
#define ASKAP_UTIL_TEST_H

#include <askap/AskapUtil.h>
#include <cppunit/extensions/HelperMacros.h>

namespace askap
{
  
  class AskapUtilTest : public CppUnit::TestFixture {
    
    CPPUNIT_TEST_SUITE(AskapUtilTest);
    CPPUNIT_TEST(testNint);
    CPPUNIT_TEST_SUITE_END();
    
  public:
    void testV(double vScaled) {
       // section of the gridder code for debugging
       const int oversample = 4;
       int iv = askap::nint(vScaled);
       int fracv=askap::nint(oversample*(double(iv)-vScaled));
        if (fracv<0) {
            iv+=1;
            //fracv+=oversample;
        }
        if (fracv>=oversample) {
            iv-=1;
            //fracv-=oversample;
        }
        fracv=askap::nint(oversample*(double(iv)-vScaled));
        ASKAPCHECK(fracv>-1, "Fractional offset in v is negative, vScaled="<<" iv="<<iv<<" oversample="<<oversample<<" fracv="<<fracv);
        ASKAPCHECK(fracv<oversample, "Fractional offset in v exceeds oversampling, vScaled="<<vScaled<<" iv="<<iv<<" oversample="<<oversample<<" fracv="<<fracv);
        //std::cerr<<vScaled<<" "<<iv<<" "<<fracv<<" "<<double(iv)-double(fracv)/double(oversample)<<std::endl;
    }
    
    void testNint() {
      const double testvals[] = {0.9, 2.2, 4.499999, 4.5,-0.1,-0.5, -3.9};
      const int results[] = {1, 2, 4, 5, 0, -1, -4};
      const size_t nVal = 7;
      for (size_t i = 0; i<nVal; ++i) {
           CPPUNIT_ASSERT(askap::nint(testvals[i]) == results[i]);
           CPPUNIT_ASSERT(askap::nint(float(testvals[i])) == results[i]);
      }
      
      testV(-272.75);
      
      for (size_t i = 0; i<200; ++i) {
          double val = -273+double(i)/100.;
          testV(val);
      }
      
    }

  };

} // namespace askap

#endif // #ifndef
