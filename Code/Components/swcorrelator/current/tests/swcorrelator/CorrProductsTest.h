/// @file
///
/// @brief Test of CorrProducts class, mainly index conversion
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

#ifndef ASKAP_SWCORRELATOR_CORR_PRODUCTS_TEST_H
#define ASKAP_SWCORRELATOR_CORR_PRODUCTS_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <askap/AskapError.h>

// Class under test
#include <swcorrelator/CorrProducts.h>

namespace askap {

namespace swcorrelator {

class CorrProductsTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(CorrProductsTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testIndexConversion);
  CPPUNIT_TEST_SUITE_END();
public:
  
  void testCreate() {
     const int nChan = 16;
     const int beam = 0;
     const int nAnt = 3;
     CorrProducts cp(nChan, beam, nAnt);
     CPPUNIT_ASSERT_EQUAL(nChan, int(cp.itsVisibility.ncolumn()));
     CPPUNIT_ASSERT_EQUAL(nChan, int(cp.itsFlag.ncolumn()));
     CPPUNIT_ASSERT_EQUAL(nAnt * (nAnt - 1) / 2, int(cp.itsVisibility.nrow()));
     CPPUNIT_ASSERT_EQUAL(nAnt * (nAnt - 1) / 2, int(cp.itsFlag.nrow()));
     CPPUNIT_ASSERT_EQUAL(nAnt * (nAnt - 1) / 2, int(cp.itsDelays.nelements()));
     CPPUNIT_ASSERT_EQUAL(nAnt * (nAnt - 1) / 2, int(cp.itsUVW.nrow()));
     CPPUNIT_ASSERT_EQUAL(3, int(cp.itsUVW.ncolumn()));
     CPPUNIT_ASSERT_EQUAL(nAnt, int(cp.itsControl.nelements()));     
  }
  
  void testIndexConversion() {
     for (int nAnt = 3; nAnt < 12; ++nAnt) {
          CorrProducts cp(16, 0, nAnt);
          for (int baseline = 0; baseline < nAnt * (nAnt - 1) / 2; ++baseline) {
               const int first = cp.first(baseline);
               const int second = cp.second(baseline);
               const int newBL = cp.baseline(first,second);
               CPPUNIT_ASSERT_EQUAL(baseline,newBL);
          }
     }
  }
};

} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_CORR_PRODUCTS_TEST_H


