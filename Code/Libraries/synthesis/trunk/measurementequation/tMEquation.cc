#include <casa/aips.h>
#include <casa/Exceptions.h>
#include <lattices/lattices/Lattice.h>
#include <lattices/lattices/LatticeExpr.h>
#include <lattices/lattices/LatticeExprNode.h>

#include <iostream.h>

#include <dataaccess/MEDataSource.h>
#include <measurementequation/MESimpleSolver.h>
#include <measurementequation/MEComponentEquation.h>
#include <measurementequation/MEParams.h>
#include <measurementequation/MEParamsTable.h>
#include <measurementequation/MEDomain.h>>
#include <measurementequation/MEQuality.h>

using namespace conrad;
 
// Someone needs these templates - I don't know who!
casa::Matrix<casa::String> c0;

int main() {
	
	try {
		// Initialize the parameters
		MEParams ip;
		ip.add("Direction.RA");
		ip.add("Direction.DEC");
		ip.add("Flux.I");
		ip.add("Flux.Q");
		ip.add("Flux.U");
		ip.add("Flux.V");
		
		MEParamsTable iptab;
		MEDomain everything;
		if(!iptab.getParameters(ip, everything)) {
			cout << "Failed to retrieve old values of parameters" << endl;
		};
		
		// The equation
		MEComponentEquation cie;
		
		// Use the simple solver
		MESimpleSolver is(ip);
		is.init();

		// Loop through data, adding equations to the solver
		MEDataSource msds;
		msds.init();
		while (msds.next()) {
			cie.calcDerivatives(ip, msds.ida(), is);
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
		
		
    } catch (casa::AipsError x) {
        cout << "Caught an exception: " << x.getMesg() << endl;
        return 1;
    } 
	return 0;
}

