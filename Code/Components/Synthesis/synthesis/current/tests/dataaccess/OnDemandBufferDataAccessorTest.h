/// @file
/// $brief Tests of the on-demand buffering adapter
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
/// 
#ifndef ON_DEMAND_BUFFER_DATA_ACCESSOR_TEST_H
#define ON_DEMAND_BUFFER_DATA_ACCESSOR_TEST_H

// cppunit includes
#include <cppunit/extensions/HelperMacros.h>
// own includes
#include <dataaccess/OnDemandBufferDataAccessor.h>
#include <dataaccess/DataAccessorStub.h>

namespace askap {

namespace synthesis {

class OnDemandBufferDataAccessorTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(OnDemandBufferDataAccessorTest);
  CPPUNIT_TEST(adapterTest);
  CPPUNIT_TEST_SUITE_END();
public:
  void adapterTest() {
      DataAccessorStub acc(true);
      checkAllCube(acc.visibility(),0.);
      OnDemandBufferDataAccessor acc2(acc);
      checkAllCube(acc2.visibility(),0.);
      acc2.rwVisibility().set(1.);
      // check that two cubes are now decoupled
      checkAllCube(acc2.visibility(),1.);
      checkAllCube(acc.visibility(),0.);
      // check they're coupled again
      acc2.discardCache();
      checkAllCube(acc2.visibility(),0.);
      //
      acc2.rwVisibility().set(2.);
      checkAllCube(acc2.visibility(),2.);
      CPPUNIT_ASSERT(acc2.nChannel()!=1); // should be 8
      // the next line should decouple cubes
      acc.rwVisibility().resize(acc.nRow(),1,acc.nPol());
      acc.rwVisibility().set(-1.);
      checkAllCube(acc.visibility(),-1.);
      checkAllCube(acc2.visibility(),-1.);      
  }
  
  /// @param[in] cube a cube to test
  /// @param[in] value all cube should have the same value
  static void checkAllCube(const casa::Cube<casa::Complex> &cube, const casa::Complex &value) {
      for (casa::uInt row = 0; row<cube.nrow(); ++row) {
           for (casa::uInt col = 0; col<cube.ncolumn(); ++col) {
                for (casa::uInt plane = 0; plane<cube.nplane(); ++plane) {
                      CPPUNIT_ASSERT(abs(cube(row,col,plane) - value)<1e-7);
                }
           }
      }
  }
  
};

} // namespace synthesis

} // namespace askap

#endif // #ifndef ON_DEMAND_BUFFER_DATA_ACCESSOR_TEST_H

