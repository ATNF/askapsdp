#include <casa/aips.h>
#include <casa/Exceptions.h>

#include <iostream.h>

#include "IEqSolver.h"
#include "IEqDataSource.h"
#include "IEqComponentEquation.h"
#include "IEqParams.h"
#include "IEqParamsTable.h"
#include "IEqDomain.h"

using namespace conrad;

// Trivial solver
class IEqTrivialSolver : public IEqSolver {
public:
	IEqTrivialSolver(const IEqParams& ip, const double gain=1.0) {
		itsParams=ip;
		itsGain=gain;
	};
	virtual void addDerivatives(IEqParams& ip) {
		itsParams.addDerivatives(ip);
	}
	virtual void init() {
		itsParams.initDerivatives();
	}
	virtual bool solve() {
		for (IEqParams::iterator iter=itsParams.begin();iter!=itsParams.end();iter++) {
			if((*iter).second.isFree()) {
				double delta, value;
				value=(*iter).second.value();
				delta=(*iter).second.deriv()/(*iter).second.deriv2();
				(*iter).second.setValue(value+itsGain*delta);
			}
		}
		return true;
	};
private:
	double itsGain;
};

// Someone needs these templates - I don't know who!
casa::Matrix<casa::String> mt;

int main() {
	
	try {
		// Initialize the parameters
		IEqParams ip;

		ip.add("Direction.RA");
		ip["Direction.RA"].fix();

		ip.add("Direction.DEC");
		ip["Direction.DEC"].fix();

		ip.add("Flux.I");
		ip.add("Flux.Q");
		ip.add("Flux.U");
		ip.add("Flux.V");
		
		cout << "Initial parameters: " << endl << ip << endl;
		
		IEqImageParams iip;
		
		// The equation
		IEqComponentEquation cie;
		
		// The solver
		IEqTrivialSolver is(ip);
		is.init();

		// Loop through data, adding equations to the solver
		IEqDataSource msds;
		msds.init();
		while (msds.next()) {
			cie.prediffer(ip, iip, msds.ida());
			is.addDerivatives(ip);
		}

		// Now we can do solution
		if(is.solve()) {
			cout << "Solution succeeded" << endl;
			IEqParamsTable iptab;
			IEqDomain everything;
			iptab.setParameters(is.getParameters(), everything);
		}
		else {
			cout << "Solution failed" << endl;
		}
		cout << "Final parameters: " << endl << is.getParameters() << endl;
		
		
    } catch (casa::AipsError x) {
        cout << "Caught an exception: " << x.getMesg() << endl;
        return 1;
    } 
	return 0;
}

