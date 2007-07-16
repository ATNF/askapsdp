/// @file TableDataAccessTest.h
/// $brief Tests of the table-based Data Accessor classes
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
/// 
#ifndef TABLE_DATA_ACCESS_TEST_H
#define TABLE_DATA_ACCESS_TEST_H

// boost includes
#include <boost/shared_ptr.hpp>

// casa includes
#include <tables/Tables/Table.h>
#include <tables/Tables/TableError.h>
#include <casa/OS/EnvVar.h>

// std includes
#include <string>
#include <vector>

// cppunit includes
#include <cppunit/extensions/HelperMacros.h>

// own includes
#include <dataaccess/DataAccessError.h>
#include <dataaccess/TableInfoAccessor.h>
#include "TableTestRunner.h"

namespace conrad {

namespace synthesis {

class TableDataAccessTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TableDataAccessTest);
  //CPPUNIT_TEST(readOnlyTest);
  CPPUNIT_TEST_EXCEPTION(bufferManagerExceptionTest,casa::TableError);
  CPPUNIT_TEST(bufferManagerTest);
  CPPUNIT_TEST(dataDescTest);
  CPPUNIT_TEST(spWindowTest);  
  CPPUNIT_TEST(feedTest);
  CPPUNIT_TEST_SUITE_END();
public:
  
  /// set up the test suite
  void setUp();
  /// destruct the test suite
  void tearDown();
  /// test of read only operations of the whole table-based implementation
  void readOnlyTest() {}
  /// test exception if disk-based buffers are requested for a read-only table  
  void bufferManagerExceptionTest();
  /// extensive test of buffer operations
  void bufferManagerTest();
  /// test access to data description subtable
  void dataDescTest();
  /// test access to spectral window subtable
  void spWindowTest();
  /// test access to the feed subtable
  void feedTest();
protected:
  void doBufferTest() const;
private:
  boost::shared_ptr<ITableInfoAccessor> itsTableInfoAccessor;  
}; // class TableDataAccessTest

void TableDataAccessTest::setUp()
{  
}

void TableDataAccessTest::tearDown()
{ 
  itsTableInfoAccessor.reset();
}

void TableDataAccessTest::bufferManagerExceptionTest()
{
  // test with the disk buffers, and leave the table read only. This
  // should throw a TableError
  itsTableInfoAccessor.reset(new TableInfoAccessor(
          casa::Table(TableTestRunner::msName()),false));
  doBufferTest();						   
}

void TableDataAccessTest::bufferManagerTest()
{
  // first test with memory buffers
  itsTableInfoAccessor.reset(new TableInfoAccessor(
              casa::Table(TableTestRunner::msName()),true));
  doBufferTest();
  // now test with the disk buffers
  itsTableInfoAccessor.reset(new TableInfoAccessor(
            casa::Table(TableTestRunner::msName(),casa::Table::Update), false));
  doBufferTest();
}

/// test access to data description subtable
void TableDataAccessTest::dataDescTest()
{
  // because we're not accessing the buffers here, it shouldn't really
  // matter whether we open it with memory buffers or with disk buffers
  // and read-only table should be enough.
  itsTableInfoAccessor.reset(new TableInfoAccessor(
              casa::Table(TableTestRunner::msName()),false));
  CONRADASSERT(itsTableInfoAccessor);
  const ITableDataDescHolder &dataDescription=itsTableInfoAccessor->
                    subtableInfo().getDataDescription();
  CPPUNIT_ASSERT(dataDescription.getSpectralWindowID(0)==0);
  CPPUNIT_ASSERT(dataDescription.getPolarizationID(0)==0);
  CPPUNIT_ASSERT(dataDescription.getDescIDsForSpWinID(0).size()==1);
  CPPUNIT_ASSERT(dataDescription.getDescIDsForSpWinID(1).size()==0);
}

/// test access to spectral window subtable
void TableDataAccessTest::spWindowTest()
{
  // because we're not accessing the buffers here, it shouldn't really
  // matter whether we open it with memory buffers or with disk buffers
  // and read-only table should be enough.
  itsTableInfoAccessor.reset(new TableInfoAccessor(
              casa::Table(TableTestRunner::msName()),false));
  CONRADASSERT(itsTableInfoAccessor);
  const ITableSpWindowHolder &spWindow=itsTableInfoAccessor->
                    subtableInfo().getSpWindow();
  CPPUNIT_ASSERT(spWindow.getReferenceFrame(0).getType() ==
                 casa::MFrequency::TOPO);
  CPPUNIT_ASSERT(spWindow.getFrequencyUnit().getName() == "Hz");
  CPPUNIT_ASSERT(spWindow.getFrequencies(0).size() == 13);
  for (casa::uInt chan=0;chan<13;++chan) { 
     CPPUNIT_ASSERT(spWindow.getFrequencies(0)[chan] ==
              spWindow.getFrequency(0,chan).getValue().getValue());
  }
  CPPUNIT_ASSERT(fabs(spWindow.getFrequencies(0)[0]-1.4e9)<1e-5);
}

/// test access to the feed subtable
void TableDataAccessTest::feedTest()
{
  // because we're not accessing the buffers here, it shouldn't really
  // matter whether we open it with memory buffers or with disk buffers
  // and read-only table should be enough.
  itsTableInfoAccessor.reset(new TableInfoAccessor(
              casa::Table(TableTestRunner::msName()),false));
  CPPUNIT_ASSERT(itsTableInfoAccessor);
  const ITableFeedHolder &feedSubtable=itsTableInfoAccessor->
                      subtableInfo().getFeed();
  casa::MEpoch time(casa::MVEpoch(casa::Quantity(50257.29,"d")),
                    casa::MEpoch::Ref(casa::MEpoch::UTC));
  for (casa::uInt feed=0; feed<5; ++feed) {                  
       for (casa::uInt ant=1; ant<6; ++ant) {
            CPPUNIT_ASSERT(fabs(feedSubtable.getBeamOffset(time,0,ant,feed)(0)-
                       feedSubtable.getBeamOffset(time,0,0,feed)(0))<1e-7);
            CPPUNIT_ASSERT(fabs(feedSubtable.getBeamOffset(time,0,ant,feed)(1)-
                       feedSubtable.getBeamOffset(time,0,0,feed)(1))<1e-7);
            CPPUNIT_ASSERT(fabs(feedSubtable.getBeamPA(time,0,ant,feed)-
                       feedSubtable.getBeamPA(time,0,0,feed))<1e-7);           
       }
       if (feed!=4) {
           CPPUNIT_ASSERT(fabs(feedSubtable.getBeamOffset(time,0,0,
                                             feed)(0))*206265-900<1e-5);
           CPPUNIT_ASSERT(fabs(feedSubtable.getBeamOffset(time,0,0,
                                             feed)(1))*206265-900<1e-5);
       } else {
           CPPUNIT_ASSERT(fabs(feedSubtable.getBeamOffset(time,0,0,
                                             feed)(0))<1e-5);
           CPPUNIT_ASSERT(fabs(feedSubtable.getBeamOffset(time,0,0,
                                             feed)(1))<1e-5);
       }
       CPPUNIT_ASSERT(fabs(feedSubtable.getBeamPA(time,0,0,feed))<1e-5);
  }                  
}


void TableDataAccessTest::doBufferTest() const
{
  const IBufferManager &bufferMgr=itsTableInfoAccessor->subtableInfo().
                                  getBufferManager();
  const casa::uInt index=5;				 
  CPPUNIT_ASSERT(!bufferMgr.bufferExists("TEST",index));
  casa::Cube<casa::Complex> vis(5,10,2);
  vis.set(casa::Complex(1.,-0.5));
  bufferMgr.writeBuffer(vis,"TEST",index);
  CPPUNIT_ASSERT(bufferMgr.bufferExists("TEST",index));
  casa::Cube<casa::Complex> vis2(5,1,2);
  vis2.set(casa::Complex(-1.,0.5));
  CPPUNIT_ASSERT(!bufferMgr.bufferExists("TEST",index-1));
  bufferMgr.writeBuffer(vis2,"TEST",index-1);
  CPPUNIT_ASSERT(bufferMgr.bufferExists("TEST",index-1));
  bufferMgr.readBuffer(vis,"TEST",index-1);
  bufferMgr.readBuffer(vis2,"TEST",index);
  CPPUNIT_ASSERT(vis.shape()==casa::IPosition(3,5,1,2));
  CPPUNIT_ASSERT(vis2.shape()==casa::IPosition(3,5,10,2));  
  for (casa::uInt x=0;x<vis.shape()[0];++x) {
       for (casa::uInt y=0;y<vis.shape()[1];++y) {
            for (casa::uInt z=0;z<vis.shape()[2];++z) {	         
	         CPPUNIT_ASSERT(abs(vis2(x,y,z)+vis(x,0,z))<1e-9);
	    }
       }
  }
}

} // namespace synthesis

} // namespace conrad

#endif // #ifndef TABLE_DATA_ACCESS_TEST_H
