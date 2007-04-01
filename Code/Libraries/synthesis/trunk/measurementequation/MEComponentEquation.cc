#include "MEComponentEquation.h"
#include "MESolver.h"

#include <msvis/MSVis/StokesVector.h>
#include <scimath/Mathematics/RigidVector.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/Complex.h>

namespace conrad
{


MEComponentEquation::~MEComponentEquation()
{
}

casa::Vector<casa::Double> MEComponentEquation::calcDelay(double ra, double dec, 
	casa::Vector<casa::RigidVector<casa::Double, 3> > uvw) {
	casa::Vector<casa::Double> delay(uvw.nelements());
	// TODO: Insert correct equation for delay
	delay=0.0;
	return delay;
}

void MEComponentEquation::predict(const MEParams& ip, MEDataAccessor& ida) {

	const double& ra=ip.regular().value("Direction.RA");
	const double& dec=ip.regular().value("Direction.DEC");
	const double& iflux=ip.regular().value("Flux.I");
	const double& qflux=ip.regular().value("Flux.Q");
	const double& uflux=ip.regular().value("Flux.U");
	const double& vflux=ip.regular().value("Flux.V");
	
	casa::CStokesVector cflux(iflux, qflux, uflux, vflux);
	cflux.applyScirc();
	
	casa::Vector<casa::Double> delay=calcDelay(ra,dec,ida.uvw());
	const casa::Vector<casa::Double>& frequency=ida.frequency();
	
	uint nChan, nRow;
	nRow=ida.nRow();
	nChan=frequency.nelements();
	
	for (int row=0;row<nRow;row++) {
		for (int chan=0;chan<nChan;chan++) {
			double phase=casa::C::pi*delay[row]*frequency[chan];
			casa::Complex phasor(cos(phase), sin(phase));
			// TODO: Need non-const version here!
//			ida.modelVisibility()(row,chan)=cflux*phasor;
		}
	}
}

void MEComponentEquation::calcDerivatives(MEParams& ip, MEDataAccessor& ida, MESolver& is) 
{
	uint nParam(ip.regular().nelements());	
	casa::LSQFit fitter(nParam);
	
	const double& ra=ip.regular().value("Direction.RA");
	const double& dec=ip.regular().value("Direction.DEC");
	const double& iflux=ip.regular().value("Flux.I");
	const double& qflux=ip.regular().value("Flux.Q");
	const double& uflux=ip.regular().value("Flux.U");
	const double& vflux=ip.regular().value("Flux.V");
	
	uint iRa=ip.regular()["Direction.RA"];
	uint iDec=ip.regular()["Direction.DEC"];
	uint iIflux=ip.regular()["Flux.I"];
	uint iQflux=ip.regular()["Flux.Q"];
	uint iUflux=ip.regular()["Flux.U"];
	uint iVflux=ip.regular()["Flux.V"];
	
	casa::Vector<casa::Double> delay=calcDelay(ra, dec, ida.uvw());
	const casa::Vector<casa::Double>& frequency=ida.frequency();
	
	casa::uInt nChan, nRow;
	nRow=ida.nRow();
	nChan=frequency.nelements();
	
	for (int row=0;row<nRow;row++) {
		vector<double> equations(2*nChan*nParam);
		vector<double> values(2*nChan);
		uint iEq=0;
		for (int chan=0;chan<nChan;chan++) {

			double phase=casa::C::pi*delay[row]*frequency[chan];
			double phasor[2];
			phasor[0]=cos(phase);
			phasor[1]=-sin(phase);

			// TODO: Pack this more robustly
//			casa::CStokesVector& rflux(ida.residualVisibility()(row,chan));
			const casa::CStokesVector& rflux(ida.visibility()(row,chan));

			equations[iEq+iIflux]=0.5*phasor[0];
			equations[iEq+iQflux]=0.5*phasor[0];
			values[iEq]=real(rflux(0));
			iEq++;

			equations[iEq+iIflux]=0.5*phasor[1];
			equations[iEq+iQflux]=0.5*phasor[1];
			values[iEq]=imag(rflux(0));
			iEq++;

			equations[iEq+iIflux]=0.5*phasor[0];
			equations[iEq+iQflux]=-0.5*phasor[0];
			values[iEq]=real(rflux(3));
			iEq++;

			equations[iEq+iIflux]=0.5*phasor[1];
			equations[iEq+iQflux]=-0.5*phasor[1];
			values[iEq]=imag(rflux(3));
			iEq++;
		}
//		fitter.makeNorm(&equations, 1.0, &values);
	}
	is.addEquations(fitter);

}

}
