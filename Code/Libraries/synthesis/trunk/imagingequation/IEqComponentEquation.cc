#include "IEqComponentEquation.h"

#include <msvis/MSVis/StokesVector.h>
#include <scimath/Mathematics/RigidVector.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/Complex.h>

namespace conrad
{

IEqComponentEquation::IEqComponentEquation(const IEqParams& ip) : IEquation(ip)
{
}

IEqComponentEquation::~IEqComponentEquation()
{
}

casa::Vector<casa::Double> IEqComponentEquation::calcDelay(double ra, double dec, 
	casa::Vector<casa::RigidVector<casa::Double, 3> > uvw) {
	casa::Vector<casa::Double> delay(uvw.nelements());
	// TODO: Insert correct equation for delay
	delay=0.0;
	return delay;
}

void IEqComponentEquation::predict(IEqDataAccessor& ida) {
	const double& ra=itsParams["Direction.RA"].value();
	const double& dec=itsParams["Direction.DEC"].value();
	const double& iflux=itsParams["Flux.I"].value();
	const double& qflux=itsParams["Flux.Q"].value();
	const double& uflux=itsParams["Flux.U"].value();
	const double& vflux=itsParams["Flux.V"].value();
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
	
	const double& rav=itsParams["Direction.RA"].value();
	const double& decv=itsParams["Direction.DEC"].value();

	casa::Vector<casa::Double> delay=calcDelay(rav, decv, ida.uvw());
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
	
	itsParams["Flux.I"].setDeriv(flux(0));
	itsParams["Flux.Q"].setDeriv(flux(1));
	itsParams["Flux.U"].setDeriv(flux(2));
	itsParams["Flux.V"].setDeriv(flux(3));
	
	itsParams["Flux.I"].setDeriv2(cfluxWeight);
	itsParams["Flux.Q"].setDeriv2(cfluxWeight);
	itsParams["Flux.U"].setDeriv2(cfluxWeight);
	itsParams["Flux.V"].setDeriv2(cfluxWeight);

	return itsParams;
}

IEqImageParams& IEqComponentEquation::transposeImage(IEqDataAccessor& ida) {
	return itsImageParams;
}

IEqParams& IEqComponentEquation::prediffer(IEqDataAccessor& ida) {
	return itsParams;
}

IEqImageParams& IEqComponentEquation::predifferImage(IEqDataAccessor& ida) {
	return itsImageParams;
}

}
