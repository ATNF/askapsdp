//
// @file tDataAccess.cc : evolving test/demonstration program of the
//                        data access layer
//

#include <dataaccess/TableDataSource.h>
#include <conrad/ConradError.h>
#include <dataaccess/SharedIter.h>

// casa
#include <measures/Measures/MFrequency.h>

// std
#include <stdexcept>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

using namespace conrad;
using namespace synthesis;

void doReadOnlyTest(const IConstDataSource &ds) {
  IDataSelectorPtr sel=ds.createSelector();
  sel->chooseFeed(1);  
  IDataConverterPtr conv=ds.createConverter();
  conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::BARY),"MHz");
  conv->setEpochFrame(casa::MEpoch(casa::Quantity(53635.5,"d"),
                      casa::MEpoch::Ref(casa::MEpoch::UTC)),"s");
  conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::AZEL));                    
  for (IConstDataSharedIter it=ds.createConstIterator(sel,conv);it!=it.end();++it) {  
       cout<<"this is a test "<<it->visibility().nrow()<<" "<<it->frequency()<<endl;
       cout<<"direction: "<<it->pointingDir2()<<endl;
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
  for (IDataSharedIter it=ds.createIterator(sel,conv);it!=it.end();++it) {
       //cout<<it.buffer("TEST").rwVisibility()<<endl;
       it.buffer("TEST").rwVisibility()=it->visibility();
       it.chooseBuffer("MODEL_DATA");
       it->rwVisibility()=it.buffer("TEST").visibility();
       it.chooseOriginal();
  }
}

int main(int argc, char **argv) {
  try {
     if (argc!=2) {
         cerr<<"Usage "<<argv[0]<<" measurement_set"<<endl;
	 return -2;
     }

     //TableDataSource ds(argv[1],TableDataSource::REMOVE_BUFFERS |
     //                           TableDataSource::MEMORY_BUFFERS);     
     TableDataSource ds(argv[1]);     
     doReadOnlyTest(ds);
     //doReadWriteTest(ds);    
     
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
