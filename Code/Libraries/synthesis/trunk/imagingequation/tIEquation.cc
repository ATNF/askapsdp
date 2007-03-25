#include <casa/aips.h>
#include <casa/Exceptions.h>

#include <iostream.h>

#include "IEqSolver.h"
#include "IEqDataSource.h"
#include "IEqComponentEquation.h"
#include "IEqParams.h"

using namespace conrad;

// Trivial solver to find peak flux
class IEqComponentSolver : public IEqSolver {
public:
	IEqComponentSolver(const IEqParams& ip) {itsParams=ip;};
	virtual void addDerivatives(IEqParams& ip) {
		itsParams.addDerivatives(ip);
	}
	virtual void init() {
		itsParams.initDerivatives();
	}
	virtual bool solve() {
		for (IEqParams::iterator iter=itsParams.begin();iter!=itsParams.end();iter++) {
			if((*iter).second.freed()) {
				(*iter).second.value()=itsParams[(*iter).first].deriv()/itsParams[(*iter).first].deriv2();
			}
		}
	};
};

int main() {
	
	try {
		// Initialize the parameters
		IEqParams ip;
		ip.add("RA");
		ip.add("DEC");
		ip.add("Flux.I");
		ip.add("Flux.Q");ip["Flux.Q"].fix();
		ip.add("Flux.U");ip["Flux.U"].fix();
		ip.add("Flux.V");ip["Flux.V"].fix();
		
		cout << "Initial parameters: " << endl << ip << endl;
		
		// The data source
		IEqDataSource msds;
		IEqComponentEquation cie(ip);
		
		IEqComponentSolver is(ip);
		is.init();
		msds.init();
		// Loop through data, adding equations
		while (msds.next()) {
			is.addDerivatives(cie.prediffer(msds.ida()));
		}
		// Now we can do solution
		if(is.solve()) {
			cout << "Solution succeeded" << endl;
			ip=is.parameters();
		}
		else {
			cout << "Solution failed" << endl;
		}
		cout << "Final parameters: " << endl << is.parameters() << endl;
		
		
    } catch (casa::AipsError x) {
        cout << "Caught an exception: " << x.getMesg() << endl;
        return 1;
    } 
	return 0;
}

