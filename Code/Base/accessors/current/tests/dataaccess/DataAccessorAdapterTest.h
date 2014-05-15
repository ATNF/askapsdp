/// @file
/// @brief Tests of the on-demand buffering adapter
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
#include <dataaccess/OnDemandNoiseAndFlagDA.h>
#include <dataaccess/DataAccessorStub.h>
#include <dataaccess/DataAccessorAdapter.h>
#include <dataaccess/TableDataSource.h>
#include <dataaccess/BestWPlaneDataAccessor.h>
#include "TableTestRunner.h"

namespace askap {

namespace accessors {

class DataAccessorAdapterTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DataAccessorAdapterTest);
  CPPUNIT_TEST(onDemandBufferDATest);
  CPPUNIT_TEST(daAdapterTest);
  CPPUNIT_TEST_EXCEPTION(daAdapterDetachTest, AskapError);
  CPPUNIT_TEST_EXCEPTION(daAdapterVoidTest, AskapError);
  CPPUNIT_TEST(daAdapterAssociationTest);
  CPPUNIT_TEST(daAdapterConstTest);
  CPPUNIT_TEST_EXCEPTION(daAdapterNonConstTest, AskapError);
  CPPUNIT_TEST(bestWPlaneAdapterTest);
  CPPUNIT_TEST_EXCEPTION(nonCoplanarTest, AskapError);
  CPPUNIT_TEST(noiseAdapterTest);
  CPPUNIT_TEST(flagAdapterTest);
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
  
  void noiseAdapterTest() {
      DataAccessorStub acc(true);
      checkAllCube(acc.noise(),1.);
      OnDemandNoiseAndFlagDA acc2(acc);
      checkAllCube(acc2.noise(),1.);
      acc2.rwNoise().set(2.);
      checkAllCube(acc2.noise(),2.);      
  }
  
  void flagAdapterTest() {
      DataAccessorStub acc(true);
      checkAllCube(acc.noise(),1.);
      checkAllBoolCube(acc.flag(), false);
      OnDemandNoiseAndFlagDA acc2(acc);
      checkAllCube(acc2.noise(),1.);
      checkAllBoolCube(acc2.flag(), false);
      acc2.rwFlag().set(casa::True);
      checkAllCube(acc2.noise(),1.);
      checkAllBoolCube(acc2.flag(), true);
      acc2.rwNoise().set(2.);
      checkAllCube(acc2.noise(),2.);                  
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
           CPPUNIT_ASSERT_DOUBLES_EQUAL(acc.feed1PA()[row], acc2.feed1PA()[row],1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(acc.feed2PA()[row], acc2.feed2PA()[row],1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(acc.uvw()[row](0), acc2.uvw()[row](0),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(acc.uvw()[row](1),acc2.uvw()[row](1), 1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(acc.uvw()[row](2), acc2.uvw()[row](2), 1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(acc.rotatedUVW(ptDir)[row](0),acc2.rotatedUVW(ptDir)[row](0),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(acc.rotatedUVW(ptDir)[row](1),acc2.rotatedUVW(ptDir)[row](1),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(acc.rotatedUVW(ptDir)[row](2),acc2.rotatedUVW(ptDir)[row](2),1e-6);
           CPPUNIT_ASSERT_DOUBLES_EQUAL(acc.uvwRotationDelay(ptDir,ptDir)[row],
                                        acc2.uvwRotationDelay(ptDir,ptDir)[row], 1e-6);
           CPPUNIT_ASSERT(acc2.pointingDir1()[row].separation(acc.pointingDir1()[row])<1e-6);
           CPPUNIT_ASSERT(acc2.pointingDir2()[row].separation(acc.pointingDir2()[row])<1e-6);
           CPPUNIT_ASSERT(acc2.dishPointing1()[row].separation(acc.dishPointing1()[row])<1e-6);
           CPPUNIT_ASSERT(acc2.dishPointing2()[row].separation(acc.dishPointing2()[row])<1e-6);
      }
      CPPUNIT_ASSERT(fabs(acc2.time() - acc.time())<1e-6);
      for (casa::uInt chan=0; chan<acc.nChannel(); ++chan) {
           CPPUNIT_ASSERT_DOUBLES_EQUAL(acc.frequency()[chan], acc2.frequency()[chan], 1e-6);           
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
  
  void bestWPlaneAdapterTest() {
      DataAccessorStub acc(true);
      makeCoplanar(acc, 1.3, -0.4);
      // we simulate only coplanar arrays in this test, so
      // tolerance of 1 wavelength should be good enough
      BestWPlaneDataAccessor acc2(1);
      scimath::ChangeMonitor cm = acc2.planeChangeMonitor();
      CPPUNIT_ASSERT(!acc2.isAssociated());
      acc2.associate(acc);
      CPPUNIT_ASSERT(acc2.isAssociated());
      testZeroW(acc2);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1.3, acc2.coeffA(), 1e-7);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.4, acc2.coeffB(), 1e-7);
      CPPUNIT_ASSERT(cm != acc2.planeChangeMonitor());
      cm = acc2.planeChangeMonitor();
      acc2.associate(acc); // technically this is not necessary
      CPPUNIT_ASSERT(acc2.isAssociated());
      // test that we still have the same plane
      testZeroW(acc2);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1.3, acc2.coeffA(), 1e-7);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.4, acc2.coeffB(), 1e-7);
      CPPUNIT_ASSERT(cm == acc2.planeChangeMonitor());
      // change the plane
      makeCoplanar(acc, -0.7, 0.5);
      acc2.associate(acc); // technically this is not necessary
      testZeroW(acc2);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.7, acc2.coeffA(), 1e-7);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, acc2.coeffB(), 1e-7);
      CPPUNIT_ASSERT(cm != acc2.planeChangeMonitor());
      cm = acc2.planeChangeMonitor();
      // make a change
      makeCoplanar(acc, -3.7, -0.05);
      testZeroW(acc2);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(-3.7, acc2.coeffA(), 1e-7);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.05, acc2.coeffB(), 1e-7);
      CPPUNIT_ASSERT(cm != acc2.planeChangeMonitor());
  }
  
  void nonCoplanarTest() 
  {
      DataAccessorStub acc(true);
      // leave w-term as it is, i.e. with full non-coplanarity
      
      // tolerance of 0.1 wavelength should be strict enough
      // to cause the exception
      BestWPlaneDataAccessor acc2(0.1);
      
      CPPUNIT_ASSERT(!acc2.isAssociated());
      acc2.associate(acc);
      CPPUNIT_ASSERT(acc2.isAssociated());
      // an exception should be thrown earlier than the result is
      // tested to be zero
      testZeroW(acc2);       
  }
  
  /// @brief a helper method to test w == 0
  /// @details After the fit, w's should be zero.
  /// @param[in] acc accessor
  static void testZeroW(const IConstDataAccessor &acc) 
  {
      CPPUNIT_ASSERT(acc.nRow() >= 1);
      const casa::MDirection fakeTangent(acc.dishPointing1()[0], casa::MDirection::J2000);
      const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw = acc.rotatedUVW(fakeTangent);      
      for (casa::uInt row=0; row<acc.nRow(); ++row) {
           CPPUNIT_ASSERT_DOUBLES_EQUAL(0,uvw[row](2),1e-7);
      }
  }
  
  /// @brief helper method to alter uvw field of the given accessor
  /// @details This method replaces w with Au+Bv making the layout 
  /// coplanar.
  /// @param[in] acc accessor 
  /// @param[in] A coefficient A
  /// @param[in] B coefficient B
  static void makeCoplanar(DataAccessorStub &acc, const double A, const double B)
  {
     for (casa::uInt row=0; row<acc.nRow(); ++row) {
          // In the stubbed class, we use itsUVW for both normal and rotated uvw
          acc.itsUVW[row](2) = A * acc.itsUVW[row](0) + B * acc.itsUVW[row](1); 
     }
  }
  
  
  /// @brief helper method to test given accessor
  /// @param[in] acc accessor  
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
  
  /// @param[in] cube a cube to test
  /// @param[in] value all cube should have the same value
  static void checkAllBoolCube(const casa::Cube<casa::Bool> &cube, const casa::Bool &value) {
      for (casa::uInt row = 0; row<cube.nrow(); ++row) {
           for (casa::uInt col = 0; col<cube.ncolumn(); ++col) {
                for (casa::uInt plane = 0; plane<cube.nplane(); ++plane) {
                      CPPUNIT_ASSERT_EQUAL(value, cube(row,col,plane));
                }
           }
      }
  }
    
};

} // namespace accessors

} // namespace askap

#endif // #ifndef DATA_ACCESSOR_ADAPTER_TEST_H

