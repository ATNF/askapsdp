#include <casa/aips.h>
#include <casa/Exceptions.h>

#include <iostream.h>

#include "IEqSolver.h"
#include "IEquation.h"
#include "IEqDataSource.h"
#include "IEqParams.h"

using namespace conrad;

typedef IEquation GSMIEquation;
typedef IEquation PeeledIEquation;
typedef IEquation MosaicIEquation;
typedef IEquation PeelingIEquation;
typedef IEqDataSource MSIEqDataSource;
typedef IEqSolver ImageIEqSolver;
	
//#include "GSMImagingEquation.h"
//#include "PeelingIEquation.h"
//#include "MosaicIEquation.h"
//#include "MSIEqDataSource.h"

// The following example makes a continuum mosaic and a 
// spectral line cube.
int main() {
	
	try {
		// The data source
		MSIEqDataSource msds("xntd.ms");
		
		// The GSM equation makes a fully accurate prediction from 
		// the global sky model including everything we know. This has 
		// no free parameters.
		IEqParams gsmparams("gsm.cl");;
		// Here we add the Global Sky Model to the list of parameters.
	//	gsmparams.add(string("gsm.cl"));
		GSMIEquation gsm("Global Sky Model Equation", gsmparams);
		
		// The peeling equation allows one real gain for the array, 
		// chunked in scans and first order in frequency. These are 
		// the only free parameters.
		IEqParams cenaparams("cena.image");
		cenaparams.add("EJones.gain", IEqParam(0.0));
		PeelingIEquation cena("Peeling Equation", cenaparams);
		
		// The mosaic equation for continuum, second order in frequency. 
		// The free parameters are images of the zeroth, first, and 
		// second derivatives.
		IEqParams cmosaicparams("cmosaic.image");
	//	cmosaic.setContext(IEquation::HIGHDYNAMICRANGE);
		MosaicIEquation cmosaic("Continuum Mosaic Equation", cmosaicparams);
		
		// The mosaic equation for spectral line, channel by channel
		IEqParams smosaicparams("smosaic.image");
	//	smosaic.setContext(IEquation::LOWDYNAMICRANGE);
		MosaicIEquation smosaic("Spectral line Mosaic Equation", smosaicparams);
		
		// Now we can solve for the unknowns in turn. We iterate to 
		// improve the solutions
		
		for (uint iteration=0;iteration<3;iteration++) {
	
			cout << "Iteration " << iteration << endl;
			
			// First we will solve for the cena gain fixing the other 
			// terms. We ignore the spectral line mosaic assuming it will 
			// have minimal effect
			ImageIEqSolver is("Image solver", cena.parameters());
			is.init();
			
			// These while loops need not be sequential and may run 
			// in different threads or processes
			// First we will solve for the gain toward Centaurus A
			msds.init();
			while (msds.next()) {
				// we need to think how to handle writable columns. A separate buffer?
				//msds.ida().initmodel();
				gsm.predict(msds.ida());
				cmosaic.predict(msds.ida());
				cena.predict(msds.ida());
				IEqParams ir=cena.transpose(msds.ida());
				is.add(ir);
			}
			if (is.solve()) {
				cena.setParameters(is.parameters());
			}
			// Now we can solve for the continuum having accounted for 
			// the global sky model and the error from Centaurus A
			ImageIEqSolver cis("Image solver", cmosaic.parameters());
			cis.init();
			msds.init();
			while (msds.next()) {
				// we need to think how to deal with writable stuff
				//msds.ida().initmodel();
				gsm.predict(msds.ida());
				cmosaic.predict(msds.ida());
				cena.predict(msds.ida());
				IEqParams ir=cmosaic.transpose(msds.ida());
				cis.add(ir);
			}
			if (cis.solve()) {
				cmosaic.setParameters(cis.parameters());
			}
			
			// Finally we can solve for the spectral line having 
			// accounted for everything else
			ImageIEqSolver sis("Image solver", smosaic.parameters());
			sis.init();
			msds.init();
			while (msds.next()) {
				// we need to think how to be with writable columns
				//msds.ida().initmodel();
				gsm.predict(msds.ida());
				cena.predict(msds.ida());
				cmosaic.predict(msds.ida());
				smosaic.predict(msds.ida());
				IEqParams ir=smosaic.transpose(msds.ida());
				sis.add(ir);
			}
			if (sis.solve()) {
				smosaic.setParameters(sis.parameters());
			}
		}
		cena.parameters().saveAsTable("cena.peeling");
    } catch (casa::AipsError x) {
        cout << "Caught an exception: " << x.getMesg() << endl;
        return 1;
    } 
	return 0;
}

