#include <casa/aips.h>
#include <casa/Exceptions.h>

#include <iostream.h>

#include "MESolver.h"
#include "MEDataSource.h"
#include "MEComponentEquation.h"
#include "MEParams.h"
#include "MEParamsTable.h"
#include "MEDomain.h"

using namespace conrad;

// Trivial solver
class METrivialSolver : public MESolver {
public:
	METrivialSolver(const MEParams& ip, const double gain=1.0) {
		itsParams=ip;
		itsGain=gain;
	};
	virtual void addDerivatives(MEParams& ip) {
		itsParams.addDerivatives(ip);
	}
	virtual void init() {
		itsParams.initDerivatives();
	}
	virtual bool solve() {
		for (MEParams::iterator iter=itsParams.begin();iter!=itsParams.end();iter++) {
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
casa::Matrix<casa::String> c0;

int main() {
	
	try {
		// Initialize the parameters
		MEParams ip;

		ip.add("Direction.RA");
		ip["Direction.RA"].fix();

		ip.add("Direction.DEC");
		ip["Direction.DEC"].fix();

		ip.add("Flux.I");
		ip.add("Flux.Q");
		ip.add("Flux.U");
		ip.add("Flux.V");
				
		MEImageParams iip;
		
		// The equation
		MEComponentEquation cie;
		
		// The solver
		METrivialSolver is(ip);
		is.init();

		// Loop through data, adding equations to the solver
		MEDataSource msds;
		msds.init();
		while (msds.next()) {
			cie.calcDerivatives(ip, iip, msds.ida(), is);
		}

		// Now we can do solution
		if(is.solve()) {
			cout << "Solution succeeded" << endl;
			MEParamsTable iptab;
			MEDomain everything;
			iptab.setParameters(is.getParameters(), everything);
		}
		else {
			cout << "Solution failed" << endl;
		}
		
		
    } catch (casa::AipsError x) {
        cout << "Caught an exception: " << x.getMesg() << endl;
        return 1;
    } 
	return 0;
}

