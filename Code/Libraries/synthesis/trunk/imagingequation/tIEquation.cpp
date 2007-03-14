#include <casa/aips.h>
#include <images/Images/PagedImage.h>
#include <components/ComponentModels/ComponentList.h>
#include <casa/OS/Path.h>

#include <iostream.h>

#include "IEquation.h"

using namespace conrad;

class GSMIEquation : public StubbedIEquation {
	public:
	GSMIEquation(const casa::ComponentList cl) : StubbedIEquation(casa::String("GSMIEquation")) {};
}
class MosaicIEquation : public StubbedIEquation {
	public:
	MosaicIEquation(const casa::PagedImage<float> im) : StubbedIEquation(casa::String("MosaicIEquation")) {};
}
class PeelingIEquation : public StubbedIEquation {
	public:
	PeelingIEquation(const casa::PagedImage<float> im) : StubbedIEquation(casa::String("PeelingIEquation")) {};
}

//#include "GSMImagingEquation.h"
//#include "PeelingIEquation.h"
//#include "MosaicIEquation.h"
//#include "MSIEqDataSource.h"

#include "IEqParams.h"

// The following example makes a continuum mosaic and a 
// spectral line cube.
int main() {
	
	// Open the existing images and the global sky model
	casa::PagedImage<float> cmosaicImage("cmosaic.image");
	casa::PagedImage<float> smosaicImage("smosaic.image");
	casa::PagedImage<float> cenaImage("cena.image");
	casa::ComponentList gsmcl(casa::Path("gsm.cl"));
		
	// The data source
	MSIEqDataSource msds("xntd.ms");
	
	// The GSM equation makes a fully accurate prediction from 
	// the global sky model including everything we know. This has 
	// no free parameters.
	GSMIEquation gsm(gsmcl);
	
	// The peeling equation allows one real gain for the array, 
	// chunked in scans and first order in frequency. These are 
	// the only free parameters.
	PeelingIEquation cena(cenaImage);
	
	// The mosaic equation for continuum, second order in frequency. 
	// The free parameters are images of the zeroth, first, and 
	// second derivatives.
	MosaicIEquation cmosaic(cmosaicImage);
	cmosaic.setContext(IEqContext::HIGHDYNAMICRANGE);
	
	// The mosaic equation for spectral line, channel by channel
	MosaicIEquation smosaic(smosaicImage);
	smosaic.setContext(IEqContext::LOWDYNAMICRANGE);
	
	// Now we can solve for the unknowns in turn. We iterate to 
	// improve the solutions
	
	for (uint iteration=0;iteration<3;iteration++) {

		// First we will solve for the cena gain fixing the other 
		// terms. We ignore the spectral line mosaic assuming it will 
		// have minimal effect
		IEqSolver is(cena.getImagingParams());
		is.init();
		
		// These while loops need not be sequential and may run 
		// in different threads or processes
		while (msds.next()) {
			msds.ida().initmodel();
			gsm.predict(msds.ida());
			cmosaic.predict(msds.ida());
			cena.predict(msds.ida());
			IEqParams ir=cena.transpose(msds.ida());
			is.add(ir);
		}
		if (is.solve()) {
			cena.setParams(is.params());
		}
		// Now we can solve for the continuum having accounted for 
		// the global sky model and the error from Centaurus A
		ImageIEqSolver cis(cmosaic.getParams());
		cis.init();
		while (msds.next()) {
			msds.ida().initmodel();
			gsm.predict(msds.ida());
			cena.predict(msds.ida());
			cmosaic.predict(msds.ida());
			IEqParams ir=cmosaic.transpose(msds.ida());
			cis.add(ir);
		}
		if (cis.solve()) {
			cmosaic.setParams(cis.param());
		}
		
		// Finally we can solve for the spectral line having 
		// accounted for everything else
		ImageImagingSolver sis(smosaic.getParams());
		sis.init();
		while (msds.next()) {
			msds.ida().initmodel();
			gsm.predict(msds.ida());
			cena.predict(msds.ida());
			cmosaic.predict(msds.ida());
			smosaic.predict(msds.ida());
			IEqParams ir=smosaic.transpose(msds.ida());
			sis.add(ir);
		}
		if (sis.solve()) {
			smosaic.setParams(sis.param());
		}
	}
	cmosaic.getParams("Image").saveAsImage(cmosaicImage);
	smosaic.getParams("Image").saveAsImage(smosaicImage);	
	cena.getParams().saveAsTable("cena.peeling");
	
	return 0;
}

