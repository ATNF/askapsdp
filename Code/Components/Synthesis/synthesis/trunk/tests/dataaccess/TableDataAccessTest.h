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
#include <dataaccess/TableDataSource.h>
#include <dataaccess/IConstDataSource.h>
#include "TableTestRunner.h"

namespace conrad {

namespace synthesis {

class TableDataAccessTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TableDataAccessTest);
  CPPUNIT_TEST(corrTypeSelectionTest);
  CPPUNIT_TEST(uvDistanceSelectionTest);
  CPPUNIT_TEST_EXCEPTION(bufferManagerExceptionTest,casa::TableError);
  CPPUNIT_TEST(bufferManagerTest);
  CPPUNIT_TEST(dataDescTest);
  CPPUNIT_TEST(spWindowTest);  
  CPPUNIT_TEST(feedTest);
  CPPUNIT_TEST(fieldTest);
  CPPUNIT_TEST(antennaTest);
  CPPUNIT_TEST(originalVisRewriteTest);
  CPPUNIT_TEST(readOnlyTest);
  CPPUNIT_TEST_SUITE_END();
public:
  
  /// set up the test suite
  void setUp();
  /// destruct the test suite
  void tearDown();
  /// test of correlation type selection
  void corrTypeSelectionTest();
  /// test of selection based on uv-distance
  void uvDistanceSelectionTest();
  /// test of read only operations of the whole table-based implementation
  void readOnlyTest();
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
  /// test access to the field subtable
  void fieldTest();
  /// test access to the antenna subtable
  void antennaTest();
  /// test to rewrite original visibilities
  void originalVisRewriteTest();
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

/// test of read only operations of the whole table-based implementation
void TableDataAccessTest::readOnlyTest()
{
  TableConstDataSource ds(TableTestRunner::msName());
 
  IDataConverterPtr conv=ds.createConverter();  
  conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::BARY),"MHz");
  conv->setEpochFrame(casa::MEpoch(casa::Quantity(50257.29,"d"),
                      casa::MEpoch::Ref(casa::MEpoch::UTC)),"s");
  conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::AZEL));                    
  
  int maxiter=4; // we don't need to go through the whole dataset as it
                 // may take a long time. A few iterations should be sufficient.   
  for (IConstDataSharedIter it=ds.createConstIterator(conv);it!=it.end();++it) {
       if (maxiter<0) {
           continue;
       }  
       --maxiter;
       // just call several accessor methods to ensure that no exception is 
       // thrown 
       it->visibility().nrow();
       it->frequency();
       it->flag();
       it->pointingDir2();
       it->antenna1();
       it->time();
  }
}


/// test of correlation type selection
void TableDataAccessTest::corrTypeSelectionTest() 
{
  TableConstDataSource ds(TableTestRunner::msName());
  IDataSelectorPtr sel = ds.createSelector();   
  sel->chooseAutoCorrelations();
  for (IConstDataSharedIter it=ds.createConstIterator(sel);it!=it.end();++it) {  
       for (casa::uInt row=0;row<it->nRow();++row) {
            CPPUNIT_ASSERT(it->antenna1()[row] == it->antenna2()[row]);
            CPPUNIT_ASSERT(it->feed1()[row] == it->feed2()[row]); 
       }
  }
  sel = ds.createSelector();
  sel->chooseCrossCorrelations();
  for (IConstDataSharedIter it=ds.createConstIterator(sel);it!=it.end();++it) {  
       for (casa::uInt row=0;row<it->nRow();++row) {
            CPPUNIT_ASSERT((it->antenna1()[row] != it->antenna2()[row]) ||
                           (it->feed1()[row] != it->feed2()[row])); 
       }
  }
}

/// test of selection based on the minimum/maximum uv distance
void TableDataAccessTest::uvDistanceSelectionTest() 
{
  TableConstDataSource ds(TableTestRunner::msName());
  IDataSelectorPtr sel = ds.createSelector();   
  sel->chooseMinUVDistance(1000.);
  for (IConstDataSharedIter it=ds.createConstIterator(sel);it!=it.end();++it) {  
       for (casa::uInt row=0;row<it->nRow();++row) {
            const casa::RigidVector<casa::Double, 3> &uvw = it->uvw()(row);
            const casa::Double uvDist = sqrt(casa::square(uvw(0))+
                                             casa::square(uvw(1)));
            CPPUNIT_ASSERT(uvDist>=1000.);                                 
       }
  }
  sel = ds.createSelector();
  sel->chooseCrossCorrelations();
  sel->chooseMaxUVDistance(3000.);
  for (IConstDataSharedIter it=ds.createConstIterator(sel);it!=it.end();++it) {  
       for (casa::uInt row=0;row<it->nRow();++row) {
            const casa::RigidVector<casa::Double, 3> &uvw = it->uvw()(row);
            const casa::Double uvDist = sqrt(casa::square(uvw(0))+
                                             casa::square(uvw(1)));
            CPPUNIT_ASSERT(uvDist<=3000.);                                 
       }
  }  
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
  const IFeedSubtableHandler &feedSubtable=itsTableInfoAccessor->
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

/// test access to the field subtable
void TableDataAccessTest::fieldTest()
{
  // because we're not accessing the buffers here, it shouldn't really
  // matter whether we open it with memory buffers or with disk buffers
  // and read-only table should be enough.
  itsTableInfoAccessor.reset(new TableInfoAccessor(
              casa::Table(TableTestRunner::msName()),false));
  CPPUNIT_ASSERT(itsTableInfoAccessor);
  const IFieldSubtableHandler &fieldSubtable=itsTableInfoAccessor->
                      subtableInfo().getField();
  casa::MEpoch time(casa::MVEpoch(casa::Quantity(50257.29,"d")),
                    casa::MEpoch::Ref(casa::MEpoch::UTC));
  casa::MVDirection refDir(casa::Quantity(0.,"deg"), casa::Quantity(-50.,"deg"));
  CPPUNIT_ASSERT(fieldSubtable.getReferenceDir(time).getRef().getType() ==
                 casa::MDirection::J2000);
  CPPUNIT_ASSERT(fieldSubtable.getReferenceDir(time).getValue().
                 separation(refDir)<1e-7);       
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
  CONRADDEBUGASSERT(vis.shape()[0]>=0);  
  CONRADDEBUGASSERT(vis.shape()[1]>=0);
  CONRADDEBUGASSERT(vis.shape()[2]>=0);
  for (casa::uInt x=0;x<casa::uInt(vis.shape()[0]);++x) {
       for (casa::uInt y=0;y<casa::uInt(vis.shape()[1]);++y) {
            for (casa::uInt z=0;z<casa::uInt(vis.shape()[2]);++z) {	         
	         CPPUNIT_ASSERT(abs(vis2(x,y,z)+vis(x,0,z))<1e-9);
	    }
       }
  }
}

/// test access to the antenna subtable
void TableDataAccessTest::antennaTest()
{
  // because we're not accessing the buffers here, it shouldn't really
  // matter whether we open it with memory buffers or with disk buffers
  // and read-only table should be enough.
  itsTableInfoAccessor.reset(new TableInfoAccessor(
              casa::Table(TableTestRunner::msName()),false));
  CPPUNIT_ASSERT(itsTableInfoAccessor);
  const IAntennaSubtableHandler &antennaSubtable=itsTableInfoAccessor->
                      subtableInfo().getAntenna();
  for (casa::uInt ant=0;ant<6;++ant) {
       CPPUNIT_ASSERT(antennaSubtable.getMount(ant) == "ALT-AZ");
      for (casa::uInt ant2=0; ant2<ant; ++ant2) {
           CPPUNIT_ASSERT(antennaSubtable.getPosition(ant).getValue().
              separation(antennaSubtable.getPosition(ant2).getValue(),"deg").
                         getValue()<0.1);
      }
  }                     
}

/// test to rewrite original visibilities
void TableDataAccessTest::originalVisRewriteTest()
{
  TableDataSource tds(TableTestRunner::msName(), TableDataSource::WRITE_PERMITTED);
  IDataSource &ds=tds; // to have all interface methods available without
                       // ambiguity (otherwise methods overridden in 
                       // TableDataSource would get a priority)
  casa::uInt iterCntr=0;
  for (IDataSharedIter it=ds.createIterator(); it!=it.end(); ++it,++iterCntr) {
       // store original visibilities in a buffer
       it.buffer("BACKUP").rwVisibility() = it->visibility();
  }
  casa::Vector<casa::Cube<casa::Complex> > memoryBuffer(iterCntr);
  iterCntr=0;
  for (IDataSharedIter it = ds.createIterator(); it!=it.end(); ++it,++iterCntr) {
       // save original values in memory to check buffers as well
       memoryBuffer[iterCntr] = it->visibility();
       // reset visibilities to a constant
       it->rwVisibility().set(casa::Complex(1.,0.5));
  }
  // check that the visibilities are set to a required constant
  for (IConstDataSharedIter cit = ds.createConstIterator(); 
                                        cit != cit.end(); ++cit) {
       const casa::Cube<casa::Complex> &vis = cit->visibility();
       for (casa::uInt row = 0; row < vis.nrow(); ++row) {
            for (casa::uInt column = 0; column < vis.ncolumn(); ++column) {
                 for (casa::uInt plane = 0; plane < vis.nplane(); ++plane) {
                      CPPUNIT_ASSERT(abs(vis(row,column,plane)-
                                         casa::Complex(1.,0.5))<1e-7);
                 }
            }
       }
  }
  // set visibilities back to the original values
  for (IDataSharedIter it=ds.createIterator(); it!=it.end(); ++it) {
       // store original visibilities in a buffer
       it->rwVisibility() = it.buffer("BACKUP").visibility();
  }
  
  // compare with the values stored in the memory
  iterCntr=0;
  for (IConstDataSharedIter cit = ds.createConstIterator(); 
                                  cit != cit.end(); ++cit,++iterCntr) {
       const casa::Cube<casa::Complex> &vis = cit->visibility();
       for (casa::uInt row = 0; row < vis.nrow(); ++row) {
            for (casa::uInt column = 0; column < vis.ncolumn(); ++column) {
                 for (casa::uInt plane = 0; plane < vis.nplane(); ++plane) {
                      CPPUNIT_ASSERT(abs(vis(row,column,plane)-
                             memoryBuffer[iterCntr](row,column,plane))<1e-7);
                 }
            }
       }
  }
}

} // namespace synthesis

} // namespace conrad

#endif // #ifndef TABLE_DATA_ACCESS_TEST_H
