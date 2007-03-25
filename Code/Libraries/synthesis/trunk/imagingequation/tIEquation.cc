#include <casa/aips.h>
#include <casa/Exceptions.h>

#include <iostream.h>

#include "IEqSolver.h"
#include "IEqDataSource.h"
#include "ComponentIEquation.h"
#include "IEqParams.h"

using namespace conrad;

// Trivial solver to find peak flux
class ComponentIEqSolver : public IEqSolver {
public:
	ComponentIEqSolver(const IEqParams& ip) {itsParams=ip;};
	virtual void addDerivatives(IEqParams& ip) {
		itsParams.addDerivatives(ip);
	}
	virtual void init() {
		itsParams.initDerivatives();
	}
	virtual bool solve() {
		itsParams["Flux.I"].value()=itsParams["Flux.I"].deriv()/itsParams["Flux.I"].deriv2();
	};
};

int main() {
	
	try {
		// Initialize the parameters
		IEqParams ip;
		ip.add("RA", IEqParam(0.0));
		ip.add("DEC", IEqParam(0.0));
		ip.add("Flux.I", IEqParam(0.0));
		ip.add("Flux.Q", IEqParam(0.0));
		ip.add("Flux.U", IEqParam(0.0));
		ip.add("Flux.V", IEqParam(0.0));
		
		// The data source
		IEqDataSource msds;
		ComponentIEquation cie(ip);
		
		ComponentIEqSolver is(ip);
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
		
    } catch (casa::AipsError x) {
        cout << "Caught an exception: " << x.getMesg() << endl;
        return 1;
    } 
	return 0;
}

