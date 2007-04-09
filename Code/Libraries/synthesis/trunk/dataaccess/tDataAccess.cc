#include <iostream>
#include <stdexcept>
#include <algorithm>

#include <casa/aips.h>
#include <casa/Exceptions.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Slice.h>


#include <boost/shared_ptr.hpp>

#include "IDataSelector.h"
#include "IConstDataIterator.h"
#include "IFlagDataAccessor.h"
#include "IDataSource.h"

using namespace conrad;
using namespace boost;
using namespace casa;
using namespace synthesis;
using namespace std;

/// Some example object-function
struct TestTransformClass {
   TestTransformClass(casa::Double il, casa::Double im) : l(il), m(im){}
   
   // some transform
   void operator()(IDataAccessor &da) const {
	for (uInt row=0;row<da.nRow();++row) {
	     // this probably can use slices instead
	     for (uInt chan=0;chan<da.nChannel();++chan) {
		  for (uInt pol=0;pol<da.nPol();++pol) {
                       da.visibility()(row,chan,pol)*=
		            exp(-2.*M_PI*Complex<Double>(0,1)*
	             (l*Double(da.uvw()[row](0))+m*Double(da.uvw()[row](1))));
		  }
	     }	  
	}
   }
private:
   casa::Double l,m;   // offsets in radians divided by wavelength (m^-1)
};


/// We don't yet have a valid implementation of the interfaces.
/// Therefore all operations have been collected inside a function 
/// (we can use just the interface here to check whether it compiles
/// and demonstrate how it is supposed to be used)


/// demonstration of flagging from the given iterator position until the
/// end of the block pointed by the iterator
void flaggingRoutine(shared_ptr<IDataIterator> &di) {
    try {
       IFlagDataAccessor &fda=dynamic_cast<IFlagDataAccessor&>(*(*di));
       for (;di->hasMore();di->next()) {
	    fda.flag()=False; // reset all flags
	    fda.flag().xyPlane(0)=True; // flag the first polarization,
	                                // whatever it is
       }
    }
    catch(const std::bad_cast &) {
	throw AipsError("flaggingRoutine - supplied DataIterator doesn't have the capability to write flagging information");
    }
}

/// demonstration of the read-only access
void readOnlyRoutine(shared_ptr<IConstDataIterator> &cdi) {
    // in this loop, start iteration from the scratch
    for (cdi->init();cdi->hasMore();cdi->next()) {

	 cout<<"UVW for row 0 ="<<(*cdi)->uvw()[0]<<" vis="<<
		 (*cdi)->visibility()(0,0,0)<<endl;
    }
}

/// demonstration of various other operations
void doTest(const shared_ptr<IDataSource> &ds) {
     // shared iterator can be empty in principle. It is good to test
     // it in high level methods
     AlwaysAssert((Bool)ds,AipsError);

     // obtain and configure data selector
     shared_ptr<IDataSelector> sel=ds->createSelector();   
     sel->chooseChannels(100,150); // 100 channels starting from 150
     sel->chooseStokes("IQUV"); // full Stokes

     // get the iterator
     shared_ptr<IDataIterator> it=ds->createIterator(sel);

     // don't need it->init() the first time, although it won't do any harm
     for (;it->hasMore();it->next()) {
         cout<<"Block has "<<(*it)->nRow()<<" rows"<<endl; 
	 // an alternative way of access
	 const IConstDataAccessor &da=*(*it);
	 cout<<"Number of channels: "<<da.nChannel()<<endl; // should be 100
     }

     // demonstration of STL, doesn't work yet
     it->init();
     //for_each(..,..,TestTransformClass(1e-4,1e-5));
     // the same thing without STL
     TestTransformClass ttc(1e-4,1e-5);
     for (;it->hasMore();it->next())
	  ttc(*(*it));
}
 
int main() {
	
    try {
       // nothing at this stage, we just check that the code is
       // compilable
		
    } catch (AipsError x) {
        cout << "Caught an exception: " << x.getMesg() << endl;
        return 1;
    } 
	return 0;
}

