#include <casa/aips.h>
#include <casa/Exceptions.h>
#include <casa/Utilities/Assert.h>

#include <iostream.h>

#include <boost/shared_ptr.hpp>
#include <casa/Exceptions.h>

#include <iostream.h>

#include <dataaccess/MEDataSource.h>
#include <dataaccess/IDataSelector.h>
#include <measurementequation/MESimpleSolver.h>
#include <measurementequation/MEComponentEquation.h>
#include <measurementequation/MEParams.h>
#include <measurementequation/MEParamsTable.h>
#include <measurementequation/MEDomain.h>>
#include <measurementequation/MEQuality.h>

using namespace conrad;
 
// Someone needs these templates - I don't know who!
casa::Matrix<casa::String> c0;

void doTest(const boost::shared_ptr<MEDataSource> &ds) {
    AlwaysAssert((bool)ds,casa::AipsError);
     
	// Initialize the parameters
	MEParams ip;
	ip.add("Direction.RA");
	ip.add("Direction.DEC");
	ip.add("Flux.I");
	
	MEParamsTable iptab;
	MEDomain everything;
	if(!iptab.getParameters(ip, everything)) {
		cout << "Failed to retrieve old values of parameters" << endl;
	};
		
	// The equation
	MEComponentEquation cie;
		
	// Use the simple solver
	MENormalEquations normeq;
	MESimpleSolver is(ip);
	is.init();

    // obtain and configure data selector
    boost::shared_ptr<IDataSelector> sel=ds->createSelector();   
    sel->chooseChannels(100,150); // 100 channels starting from 150
    sel->chooseStokes("IQUV");

    // get the iterator
    boost::shared_ptr<MEDataIterator> it=ds->createIterator(sel);
	// Loop through data, adding equations to the solver
	for (;it->hasMore();it->next()) {
		// Won't compile until we have a writable accessor
//		cie.calcNormalEquations(ip, *(*it), normeq);
		is.addEquations(normeq);
	}

	// Now we can do solution
	MEQuality quality;
	if(is.solve(quality)) {
		cout << "Solution succeeded" << endl;
		iptab.setParameters(is.getParameters(), everything);
	}
	else {
		cout << "Solution failed" << endl;
	}
}

int main() {
	
    try {
       // nothing at this stage, we just check that the code is
       // compilable
		
    } catch (casa::AipsError x) {
        cout << "Caught an exception: " << x.getMesg() << endl;
        return 1;
    } 
	return 0;
}


