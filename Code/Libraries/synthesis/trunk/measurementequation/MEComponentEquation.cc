#include <dataaccess/MEDataAccessor.h>
#include <measurementequation/MEParams.h>
#include <measurementequation/MEComponentEquation.h>
#include <measurementequation/MENormalEquations.h>

#include <msvis/MSVis/StokesVector.h>
#include <scimath/Mathematics/RigidVector.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <scimath/Mathematics/AutoDiff.h>
#include <scimath/Mathematics/AutoDiffMath.h>



namespace conrad
{

MEComponentEquation::~MEComponentEquation()
{
}

void MEComponentEquation::predict(const MEParams& ip, MEDataAccessor& ida) 
{
	// Get the data from the accessor
	casa::Vector<double> vreal;
	casa::Vector<double> vimag;
	// ...
	
	// Calculate values and derivatives
	this->calc<double>(ida, ip, vreal, vimag);
	
	// Put the values back into the accessor
	// ...
}

void MEComponentEquation::calcNormalEquations(MEParams& ip, MEDataAccessor& ida, 
	MENormalEquations& normeq) 
{
	// Get the data from the accessor
	casa::Vector<casa::AutoDiff<double> > vreal;
	casa::Vector<casa::AutoDiff<double> > vimag;
	// ...
	
	// Calculate values and derivatives
	this->calc<casa::AutoDiff<double> >(ida, ip, vreal, vimag);
	
	// Put the values back into the accessor
	// ...
	
	// Put the derivatives into the normal equations
	// ...
}

template<class T>
void MEComponentEquation::calc(const MEDataAccessor& ida, const MEParams& ip, 
	casa::Vector<T>& vreal, casa::Vector<T>& vimag) {
	
	const T& ra=ip.regular().value("DIRECTION.RA");
	const T& dec=ip.regular().value("DIRECTION.DEC");
	const T& flux=ip.regular().value("FLUX.I");

	const casa::Vector<double>& freq=ida.frequency();	
	
	for (uint row=0;row<ida.nRow();row++) {
		const double& u=ida.uvw()(row)(0);
		const double& v=ida.uvw()(row)(1);
		T  delay;
		delay = casa::C::_2pi * (ra * u + dec * v)/casa::C::c;
		casa::Vector<T> phase(freq.nelements());
		for (uint i=0;i<freq.nelements();i++) {
			phase(i) = delay * freq(i);
		}
		vreal=flux*cos(phase);
		vimag=flux*sin(phase);
	}
};

}
// Declare necessary templates
#include <casa/Arrays/ArrayMath.cc>
template casa::Array<casa::AutoDiff<double> > casa::cos<casa::AutoDiff<double> >(casa::Array<casa::AutoDiff<double> > const&);
template casa::Array<casa::AutoDiff<double> > casa::sin<casa::AutoDiff<double> >(casa::Array<casa::AutoDiff<double> > const&);
template casa::Array<casa::AutoDiff<double> > casa::operator*<casa::AutoDiff<double> >(casa::AutoDiff<double> const&, casa::Array<casa::AutoDiff<double> > const&);

