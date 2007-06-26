//
// @file tDataAccess.cc : evolving test/demonstration program of the
//                        data access layer
//

#include <dataaccess/TableConstDataSource.h>
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
  conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),"MHz");
  conv->setEpochFrame(casa::MEpoch(casa::Quantity(53635.5,"d"),
                      casa::MEpoch::Ref(casa::MEpoch::UTC)),"s");
  for (IConstDataSharedIter it=ds.createConstIterator(sel,conv);it!=it.end();++it) {  
       cout<<"this is a test "<<it->visibility().nrow()<<" "<<it->frequency()<<endl;
       cout<<"uvw: "<<it->uvw()(1)<<endl;
       cout<<"time: "<<it->time()<<endl;
  }
}

int main(int argc, char **argv) {
  try {
     if (argc!=2) {
         cerr<<"Usage "<<argv[0]<<" measurement_set"<<endl;
	 return -2;
     }

     TableConstDataSource ds(argv[1]);
     doReadOnlyTest(ds);    
     
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
