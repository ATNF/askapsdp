//
// @file tDataAccess.cc : evolving test/demonstration program of the
//                        data access layer
//

#include <dataaccess/TableDataSource.h>
#include <conrad_synthesis.h>
#include <conrad/ConradLogging.h>
#include <conrad/ConradError.h>
#include <dataaccess/SharedIter.h>
#include <dataaccess/ParsetInterface.h>

#include <dataaccess/TableManager.h>
#include <dataaccess/IDataConverterImpl.h>

// casa
#include <measures/Measures/MFrequency.h>
#include <tables/Tables/Table.h>

// std
#include <stdexcept>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

using namespace conrad;
using namespace synthesis;

void timeDependentSubtableTest(const string &ms, const IConstDataSource &ds) 
{
  IDataConverterPtr conv=ds.createConverter();  
  //conv->setEpochFrame(casa::MEpoch(casa::Quantity(53635.5,"d"),
  //                    casa::MEpoch::Ref(casa::MEpoch::UTC)),"s");
  IDataSelectorPtr sel=ds.createSelector();
  //sel->chooseFeed(1);
  //sel<<LOFAR::ACC::APS::ParameterSet("test.in").makeSubset("TestSelection.");
  const IDataConverterImpl &dci=dynamic_cast<const IDataConverterImpl&>(*conv);
  const TableManager tm(casa::Table(ms),true);
  const IFeedSubtableHandler &fsh = tm.getFeed();  
  for (IConstDataSharedIter it=ds.createConstIterator(sel,conv);it!=it.end();++it) {  
       cout<<"direction: "<<it->pointingDir2()<<endl;
       cout<<"time: "<<it->time()<<" "<<dci.epochMeasure(it->time())<<" "<<
             fsh.getAllBeamOffsets(dci.epochMeasure(it->time()),0)<<endl;
  }
}

void doReadOnlyTest(const IConstDataSource &ds) {
  IDataSelectorPtr sel=ds.createSelector();
  //sel->chooseFeed(1);
  //sel<<LOFAR::ACC::APS::ParameterSet("test.in").makeSubset("TestSelection.");
  IDataConverterPtr conv=ds.createConverter();  
  conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::BARY),"MHz");
  conv->setEpochFrame(casa::MEpoch(casa::Quantity(53635.5,"d"),
                      casa::MEpoch::Ref(casa::MEpoch::UTC)),"s");
  conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::AZEL));                    
    
  for (IConstDataSharedIter it=ds.createConstIterator(sel,conv);it!=it.end();++it) {  
       cout<<"this is a test "<<it->visibility().nrow()<<" "<<it->frequency()<<endl;
       //cout<<"direction: "<<it->pointingDir2()<<endl;
       cout<<"ant1: "<<it->antenna1()<<endl;
       cout<<"ant2: "<<it->antenna2()<<endl;
       cout<<"time: "<<it->time()<<endl;
  }
}

void doReadWriteTest(const IDataSource &ds) {
  IDataSelectorPtr sel=ds.createSelector();
  sel->chooseFeed(1);  
  IDataConverterPtr conv=ds.createConverter();
  conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),"MHz");
  conv->setEpochFrame(casa::MEpoch(casa::Quantity(53635.5,"d"),
                      casa::MEpoch::Ref(casa::MEpoch::UTC)),"s");
  IDataSharedIter it=ds.createIterator(sel,conv);
  //for (size_t run=0;run<10;++run)
  for (it.init();it!=it.end();it.next()) {
       //cout<<it.buffer("TEST").rwVisibility()<<endl;
       it->frequency();
       it->pointingDir1();
       it->time();
       it->antenna1();
       it->feed1();
       it->uvw();
       it.buffer("TEST").rwVisibility()=it->visibility();
       it.chooseBuffer("MODEL_DATA");
       it->rwVisibility()=it.buffer("TEST").visibility();
       it.chooseOriginal();
       it->rwVisibility().set(casa::Complex(1.,0.5));
  }
}

int main(int argc, char **argv) {
  try {
     if (argc!=2) {
         cerr<<"Usage "<<argv[0]<<" measurement_set"<<endl;
	 return -2;
     }

     TableDataSource ds(argv[1],TableDataSource::REMOVE_BUFFERS |
                                TableDataSource::MEMORY_BUFFERS);     
     //TableDataSource ds(argv[1],TableDataSource::MEMORY_BUFFERS);     
     //timeDependentSubtableTest(argv[1],ds);
     //doReadOnlyTest(ds);
     doReadWriteTest(ds);    
     
  }
  catch(const ConradError &ce) {
     cerr<<"ConradError has been caught. "<<ce.what()<<endl;
     return -1;
  }
  catch(const std::exception &ex) {
     cerr<<"std::exception has been caught. "<<ex.what()<<endl;
     return -1;
  }
  catch(...) {
     cerr<<"An unexpected exception has been caught"<<endl;
     return -1;
  }
  return 0;
}
