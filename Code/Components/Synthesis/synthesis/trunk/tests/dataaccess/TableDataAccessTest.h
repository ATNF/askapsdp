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

namespace conrad {

namespace synthesis {

class TableDataAccessTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TableDataAccessTest);
  //CPPUNIT_TEST(readOnlyTest);
  CPPUNIT_TEST_EXCEPTION(bufferManagerExceptionTest,casa::TableError);
  CPPUNIT_TEST(bufferManagerTest);
  //CPPUNIT_TEST(dataDescTest);  
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
protected:
  void doBufferTest() const;
private:
  std::string itsTestMSName;
  boost::shared_ptr<ITableInfoAccessor> itsTableInfoAccessor;  
}; // class TableDataAccessTest

void TableDataAccessTest::setUp()
{  
  itsTestMSName="./.test.ms";
  std::string path2TestMS="../../testdata/trunk/testdataset.ms";
  if (casa::EnvironmentVariable::isDefined("CONRAD_PROJECT_ROOT")) {
      path2TestMS=casa::EnvironmentVariable::get("CONRAD_PROJECT_ROOT")+
                "/Code/Components/Synthesis/testdata/trunk/testdataset.ms";
  }
  try {
    casa::Table originalMS(path2TestMS);
    originalMS.deepCopy(itsTestMSName,casa::Table::New);
  }
  catch (const casa::AipsError &ae) {
      CONRADTHROW(DataAccessError, "The current directory is not writable, can't "<<
                  "create a copy of the test dataset. AipsError: "<<ae.what());
  }  
}

void TableDataAccessTest::tearDown()
{ 
  itsTableInfoAccessor.reset();
  casa::Table copiedMS(itsTestMSName,casa::Table::Update);
  copiedMS.markForDelete();  
}

void TableDataAccessTest::bufferManagerExceptionTest()
{
  // test with the disk buffers, and leave the table read only. This
  // should throw a TableError
  itsTableInfoAccessor.reset(new TableInfoAccessor(casa::Table(itsTestMSName),
                                                   false));
  doBufferTest();						   
}

void TableDataAccessTest::bufferManagerTest()
{
  // first test with memory buffers
  itsTableInfoAccessor.reset(new TableInfoAccessor(casa::Table(itsTestMSName),
                                                  true));
  doBufferTest();
  // now test with the disk buffers, and leave this set for other tests
  itsTableInfoAccessor.reset(new TableInfoAccessor(casa::Table(itsTestMSName,
                                   casa::Table::Update), false));
  doBufferTest();
}

/// test access to data description subtable
void TableDataAccessTest::dataDescTest()
{
  CONRADASSERT(itsTableInfoAccessor);
  const ITableDataDescHolder &dataDescription=itsTableInfoAccessor->
                    subtableInfo().getDataDescription();
  CPPUNIT_ASSERT(dataDescription.getSpectralWindowID(0)==0);
  CPPUNIT_ASSERT(dataDescription.getPolarizationID(0)==0);
  CPPUNIT_ASSERT(dataDescription.getDescIDsForSpWinID(0).size()==1);
  CPPUNIT_ASSERT(dataDescription.getDescIDsForSpWinID(1).size()==0);
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
