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
#ifndef DATA_ACCESSOR_ADAPTER_TEST_H
#define DATA_ACCESSOR_ADAPTER_TEST_H

// cppunit includes
#include <cppunit/extensions/HelperMacros.h>
// own includes
#include <dataaccess/OnDemandBufferDataAccessor.h>
#include <dataaccess/DataAccessorStub.h>
#include <dataaccess/DataAccessorAdapter.h>
#include <dataaccess/TableDataSource.h>
#include "TableTestRunner.h"

namespace askap {

namespace synthesis {

class DataAccessorAdapterTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DataAccessorAdapterTest);
  CPPUNIT_TEST(onDemandBufferDATest);
  CPPUNIT_TEST(daAdapterTest);
  CPPUNIT_TEST_EXCEPTION(daAdapterDetachTest, AskapError);
  CPPUNIT_TEST_EXCEPTION(daAdapterVoidTest, AskapError);
  CPPUNIT_TEST(daAdapterAssociationTest);
  CPPUNIT_TEST(daAdapterConstTest);
  CPPUNIT_TEST_EXCEPTION(daAdapterNonConstTest, AskapError);
  CPPUNIT_TEST_SUITE_END();
public:
  void onDemandBufferDATest() {
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
  
  void daAdapterTest() {
      DataAccessorStub acc(true);
      checkAllCube(acc.visibility(),0.);      
      DataAccessorAdapter acc2(acc);
      checkAllCube(acc2.visibility(),0.);      
      acc2.rwVisibility().set(1.);
      // check that two cubes are coupled together
      CPPUNIT_ASSERT(acc2.isAssociated());
      CPPUNIT_ASSERT(acc2.nRow() == acc.nRow());
      CPPUNIT_ASSERT(acc.nRow()>=1);
      CPPUNIT_ASSERT(acc2.nChannel() == acc.nChannel());
      CPPUNIT_ASSERT(acc2.nPol() == acc.nPol());
      checkAllCube(acc2.visibility(),1.);
      checkAllCube(acc.visibility(),1.);
      checkAllCube(acc2.noise(),1.);
      checkAllCube(acc.noise(),1.);      
      const casa::MDirection ptDir(acc.dishPointing1()[0],casa::MDirection::J2000);
            
      for (casa::uInt row=0;row<acc.nRow(); ++row) {
           CPPUNIT_ASSERT(acc2.feed1()[row] == acc.feed1()[row]);
           CPPUNIT_ASSERT(acc2.feed2()[row] == acc.feed2()[row]);           
           CPPUNIT_ASSERT(acc2.antenna1()[row] == acc.antenna1()[row]);
           CPPUNIT_ASSERT(acc2.antenna2()[row] == acc.antenna2()[row]);           
           CPPUNIT_ASSERT(fabs(acc2.feed1PA()[row] - acc.feed1PA()[row])<1e-6);
           CPPUNIT_ASSERT(fabs(acc2.feed2PA()[row] - acc.feed2PA()[row])<1e-6);
           CPPUNIT_ASSERT(fabs(acc2.uvw()[row](0) - acc.uvw()[row](0))<1e-6);
           CPPUNIT_ASSERT(fabs(acc2.uvw()[row](1) - acc.uvw()[row](1))<1e-6);
           CPPUNIT_ASSERT(fabs(acc2.uvw()[row](2) - acc.uvw()[row](2))<1e-6);
           CPPUNIT_ASSERT(fabs(acc2.rotatedUVW(ptDir)[row](0) - acc.rotatedUVW(ptDir)[row](0))<1e-6);
           CPPUNIT_ASSERT(fabs(acc2.rotatedUVW(ptDir)[row](1) - acc.rotatedUVW(ptDir)[row](1))<1e-6);
           CPPUNIT_ASSERT(fabs(acc2.rotatedUVW(ptDir)[row](2) - acc.rotatedUVW(ptDir)[row](2))<1e-6);
           CPPUNIT_ASSERT(fabs(acc2.uvwRotationDelay(ptDir,ptDir)[row] - acc.uvwRotationDelay(ptDir,ptDir)[row])<1e-6);
           CPPUNIT_ASSERT(acc2.pointingDir1()[row].separation(acc.pointingDir1()[row])<1e-6);
           CPPUNIT_ASSERT(acc2.pointingDir2()[row].separation(acc.pointingDir2()[row])<1e-6);
           CPPUNIT_ASSERT(acc2.dishPointing1()[row].separation(acc.dishPointing1()[row])<1e-6);
           CPPUNIT_ASSERT(acc2.dishPointing2()[row].separation(acc.dishPointing2()[row])<1e-6);
      }
      CPPUNIT_ASSERT(fabs(acc2.time() - acc.time())<1e-6);
      for (casa::uInt chan=0; chan<acc.nChannel(); ++chan) {
           CPPUNIT_ASSERT(fabs(acc2.frequency()[chan] - acc.frequency()[chan])<1e-6);           
      }
      for (casa::uInt pol=0; pol<acc.nPol(); ++pol) {
           CPPUNIT_ASSERT(acc2.stokes()[pol] == acc.stokes()[pol]);      
      }
      
      acc2.detach();
      CPPUNIT_ASSERT(!acc2.isAssociated());      
  }
  
  void daAdapterDetachTest() {
      DataAccessorStub acc(true);
      checkAllCube(acc.visibility(),0.);      
      DataAccessorAdapter acc2(acc);
      checkAllCube(acc2.visibility(),0.);      
      acc2.detach();
      CPPUNIT_ASSERT(!acc2.isAssociated());      
      // the following line should throw an exception
      checkAllCube(acc2.visibility(),0.);      
  }
  
  void daAdapterVoidTest() {
      DataAccessorAdapter acc;
      CPPUNIT_ASSERT(!acc.isAssociated());
      // the following line should throw an exception
      acc.rwVisibility().set(1.0);      
  }
  
  void daAdapterAssociationTest() {
      DataAccessorAdapter acc2;
      CPPUNIT_ASSERT(!acc2.isAssociated());
      DataAccessorStub acc(true);
      checkAllCube(acc.visibility(),0.);      
      acc2.associate(acc);
      CPPUNIT_ASSERT(acc2.isAssociated());
      acc2.rwVisibility().set(1.0);      
      checkAllCube(acc.visibility(),1.);      
      checkAllCube(acc2.visibility(),1.);      
  }
  
  void daAdapterConstTest() {
      TableConstDataSource ds(TableTestRunner::msName());
      IConstDataSharedIter it = ds.createConstIterator();
      CPPUNIT_ASSERT(it != it.end());
      DataAccessorAdapter acc2(*it);
      doConstAccessTest(*it);
      CPPUNIT_ASSERT(acc2.isAssociated());
      doConstAccessTest(acc2);         
  }
  void daAdapterNonConstTest() {
      TableConstDataSource ds(TableTestRunner::msName());
      IConstDataSharedIter it = ds.createConstIterator();
      CPPUNIT_ASSERT(it != it.end());
      DataAccessorAdapter acc2(*it);
      doConstAccessTest(*it);
      CPPUNIT_ASSERT(acc2.isAssociated());
      doConstAccessTest(acc2);         
      // the following line should generate an exception
      // because we have a const accessor
      acc2.rwVisibility().set(1.);
  }
  
  static void doConstAccessTest(const IConstDataAccessor &acc) 
  {
      CPPUNIT_ASSERT(acc.visibility().nrow() == acc.antenna1().nelements());
      CPPUNIT_ASSERT(acc.visibility().ncolumn() == acc.frequency().nelements());
      CPPUNIT_ASSERT(acc.visibility().nplane() == acc.stokes().nelements());
      CPPUNIT_ASSERT(acc.antenna1().nelements() == acc.feed2().nelements());
      CPPUNIT_ASSERT(acc.noise().shape() == acc.visibility().shape());
      CPPUNIT_ASSERT(acc.nPol() == 2);
      CPPUNIT_ASSERT(acc.stokes()[0] == casa::Stokes::XX);
      CPPUNIT_ASSERT(acc.stokes()[1] == casa::Stokes::YY);      
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

#endif // #ifndef DATA_ACCESSOR_ADAPTER_TEST_H

