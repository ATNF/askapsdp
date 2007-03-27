#include "IEqComponentEquation.h"

#include <msvis/MSVis/StokesVector.h>
#include <scimath/Mathematics/RigidVector.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/Complex.h>

namespace conrad
{


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

void IEqComponentEquation::predict(const IEqParams& ip, const IEqImageParams& iip, IEqDataAccessor& ida) {
	const double& ra=ip("Direction.RA").value();
	const double& dec=ip("Direction.DEC").value();
	const double& iflux=ip("Flux.I").value();
	const double& qflux=ip("Flux.Q").value();
	const double& uflux=ip("Flux.U").value();
	const double& vflux=ip("Flux.V").value();
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

void IEqComponentEquation::transpose(IEqParams& ip, const IEqImageParams& iip,
	IEqDataAccessor& ida) {

	casa::StokesVector flux(0.0);
	double cfluxWeight=0.0;
	
	const double& rav=ip("Direction.RA").value();
	const double& decv=ip("Direction.DEC").value();

	casa::Vector<casa::Double> delay=calcDelay(rav, decv, ida.uvw());
	const casa::Vector<casa::Double>& frequency=ida.frequency();
	
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
	
	ip("Flux.I").setDeriv(flux(0));
	ip("Flux.Q").setDeriv(flux(1));
	ip("Flux.U").setDeriv(flux(2));
	ip("Flux.V").setDeriv(flux(3));
	
	ip("Flux.I").setDeriv2(cfluxWeight);
	ip("Flux.Q").setDeriv2(cfluxWeight);
	ip("Flux.U").setDeriv2(cfluxWeight);
	ip("Flux.V").setDeriv2(cfluxWeight);

}

void IEqComponentEquation::transposeImage(const IEqParams& ip, IEqImageParams& iip, IEqDataAccessor& ida) {
}

void IEqComponentEquation::prediffer(IEqParams& ip, const IEqImageParams& iip, IEqDataAccessor& ida) {
}

void IEqComponentEquation::predifferImage(const IEqParams& ip, IEqImageParams& iip, IEqDataAccessor& ida) {
}

}
