#include "IEqComponentEquation.h"

#include <msvis/MSVis/StokesVector.h>
#include <scimath/Mathematics/RigidVector.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/Complex.h>

namespace conrad
{

IEqComponentEquation::IEqComponentEquation(IEqParams& ip) 
{
	itsParams=ip;
}

IEqComponentEquation::~IEqComponentEquation()
{
}

casa::Vector<casa::Double> IEqComponentEquation::calcDelay(double ra, double dec, 
	casa::Vector<casa::RigidVector<casa::Double, 3> > uvw) {
	casa::Vector<casa::Double> delay(uvw.nelements());
	delay=0.0;
	return delay;
}

void IEqComponentEquation::predict(IEqDataAccessor& ida) {
	double& ra=itsParams["RA"].value();
	double& dec=itsParams["DEC"].value();
	double& iflux=itsParams["Flux.I"].value();
	double& qflux=itsParams["Flux.Q"].value();
	double& uflux=itsParams["Flux.U"].value();
	double& vflux=itsParams["Flux.V"].value();
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
//			ida.visibility()(row,chan)=cflux*phasor;
		}
	}
}

IEqParams& IEqComponentEquation::transpose(IEqDataAccessor& ida) {
	
	casa::StokesVector flux(0.0);
	double cfluxWeight=0.0;
	
	double& rav=itsParams["RA"].value();
	double& decv=itsParams["DEC"].value();

	casa::Vector<casa::Double> delay=calcDelay(rav, decv,ida.uvw());
	const casa::Vector<casa::Double>& frequency=ida.frequency();
	
	IEqParam ra, dec, iflux, qflux, uflux, vflux;

	casa::uInt nChan, nRow;
	nRow=ida.nRow();
	nChan=frequency.nelements();
	
	for (int row=0;row<nRow;row++) {
		for (int chan=0;chan<nChan;chan++) {
			double phase=casa::C::pi*delay[row]*frequency[chan];
			casa::Complex phasor(cos(phase), -sin(phase));
			// TODO: weights and phasor!
			casa::CStokesVector cflux(ida.visibility()(row,chan));
			flux+=casa::applyScircInv(cflux);
			cfluxWeight=cfluxWeight+1.0;
		}
	}
	
	itsParams["Flux.I"].deriv()=flux(0);
	itsParams["Flux.Q"].deriv()=flux(1);
	itsParams["Flux.U"].deriv()=flux(2);
	itsParams["Flux.V"].deriv()=flux(3);
	
	itsParams["Flux.I"].deriv2()=cfluxWeight;
	itsParams["Flux.Q"].deriv2()=cfluxWeight;
	itsParams["Flux.U"].deriv2()=cfluxWeight;
	itsParams["Flux.V"].deriv2()=cfluxWeight;

	return itsParams;
}

IEqParams& IEqComponentEquation::prediffer(IEqDataAccessor& ida) {
}

}
