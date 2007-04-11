#include <casa/aips.h>
#include <casa/Exceptions.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>

#include <iostream.h>

#include <boost/shared_ptr.hpp>
#include <casa/Exceptions.h>

#include <iostream.h>

#include <dataaccess/IDataSource.h>
#include <dataaccess/IDataSelector.h>
#include <dataaccess/IDataAccessor.h>
#include <dataaccess/IDataIterator.h>
#include <measurementequation/MESimpleSolver.h>
#include <measurementequation/MEComponentEquation.h>
#include <measurementequation/MEParams.h>
#include <measurementequation/MEParamsTable.h>
#include <measurementequation/MEDomain.h>>
#include <measurementequation/MEQuality.h>

using namespace conrad::synthesis;
 
// Someone needs these templates - I don't know who!
casa::Matrix<casa::String> s0;

void doTest(const boost::shared_ptr<IDataSource> &ds) 
{
//    casa::AlwaysAssert((bool)ds,casa::AipsError);
     
	// Declare the equation with no parameters so we can get the
	// default values.
	MEComponentEquation cie;
	MEParams ip(cie.defaultParameters());
	
	// Use the simple solver
	MERegularNormalEquations normeq(ip.regular());
	MERegularDesignMatrix designmatrix(ip.regular());
	MESimpleSolver is(ip.regular());
	is.init();

    // obtain and configure data selector
    boost::shared_ptr<IDataSelector> sel=ds->createSelector();   
    sel->chooseChannels(100,150); // 100 channels starting from 150
    sel->chooseStokes("IQUV");

    // get the iterator
    boost::shared_ptr<IDataIterator> it=ds->createIterator(sel);
	// Loop through data, adding equations to the solver
	for (;it->hasMore();it->next()) {
		cie.calcEquations(*(*it), normeq);
		is.addNormalEquations(normeq);
	}

	// Now we can do solution
	MEQuality quality;
	if(is.solveNormalEquations(quality)) {
		MEParamsTable iptab;
		MEDomain everything;
		cout << "Solution succeeded" << endl;
//		iptab.setParameters(is.getParameters(), everything);
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


