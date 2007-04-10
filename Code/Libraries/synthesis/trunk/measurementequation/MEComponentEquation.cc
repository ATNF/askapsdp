#include <dataaccess/IDataAccessor.h>
#include <measurementequation/MEParams.h>
#include <measurementequation/MEComponentEquation.h>
#include <measurementequation/MENormalEquations.h>
#include <measurementequation/MEDesignMatrix.h>

#include <msvis/MSVis/StokesVector.h>
#include <scimath/Mathematics/RigidVector.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/ArrayMath.h>
#include <scimath/Mathematics/AutoDiff.h>
#include <scimath/Mathematics/AutoDiffMath.h>



namespace conrad
{
namespace synthesis
{

MEComponentEquation::~MEComponentEquation()
{
}

void MEComponentEquation::init()
{
	itsDefaultParams.reset();
	itsDefaultParams.add("flux.i.*");
	itsDefaultParams.add("direction.ra.*");
	itsDefaultParams.add("direction.dec.*");
}

void MEComponentEquation::predict(IDataAccessor& ida) 
{ 
	const casa::Vector<double>& freq=ida.frequency();	
	const casa::Vector<double>& time=ida.time();	
	vector<string> completions(parameters().regular().completions("flux.i.*"));
	vector<string>::iterator it;
		
	for (uint row=0;row<ida.nRow();row++) {

		// The loop has to be this order so that we get the derivatives with respect
		// to all parameters
		for (it=completions.begin();it!=completions.end();it++) {
	
			const double& ra=parameters().regular().value("direction.ra."+(*it));
			const double& dec=parameters().regular().value("direction.dec."+(*it));
			const double& flux=parameters().regular().value("flux.i."+(*it));

			const double& u=ida.uvw()(row)(0);
			const double& v=ida.uvw()(row)(1);
			casa::Vector<float> vreal(freq.nelements());
			casa::Vector<float> vimag(freq.nelements());
			this->calcVis<float>(ra, dec, flux, freq, u, v, vreal, vimag);

			for (uint i=0;i<freq.nelements();i++) {
				real(ida.visibility()(row,i,0)) += vreal(i);
				imag(ida.visibility()(row,i,0)) += vimag(i);
			}
		}
	}
};

void MEComponentEquation::calcEquations(IDataAccessor& ida, 
MERegularNormalEquations& normeq) 
{
	MERegularDesignMatrix designmatrix(parameters().regular());
	calcEquations(ida, designmatrix, normeq);	
}

void MEComponentEquation::calcEquations(IDataAccessor& ida, 
	MERegularDesignMatrix& designmatrix, MERegularNormalEquations& normeq) 
{
	calcEquations(ida, designmatrix);	
	// Put the derivatives into the normal equations
	// ...
}

void MEComponentEquation::calcEquations(IDataAccessor& ida, MERegularDesignMatrix& designmatrix) 
{
	const casa::Vector<double>& freq=ida.frequency();	
	const casa::Vector<double>& time=ida.time();	
	vector<string> completions(parameters().regular().completions("flux.i.*"));
	vector<string>::iterator it;
		
	for (uint row=0;row<ida.nRow();row++) {

		// The loop has to be this order so that we get the derivatives with respect
		// to all parameters
		for (it=completions.begin();it!=completions.end();it++) {
	
			const double& ra=parameters().regular().value("direction.ra."+(*it));
			const double& dec=parameters().regular().value("direction.dec."+(*it));
			const double& flux=parameters().regular().value("flux.i."+(*it));

			const double& u=ida.uvw()(row)(0);
			const double& v=ida.uvw()(row)(1);
			casa::Vector<float> vreal(freq.nelements());
			casa::Vector<float> vimag(freq.nelements());
			this->calcVis<float>(ra, dec, flux, freq, u, v, vreal, vimag);
			for (uint i=0;i<freq.nelements();i++) {
				real(ida.visibility()(row,i,0)) += vreal(i);
				imag(ida.visibility()(row,i,0)) += vimag(i);
			}
		}
	}
};

template<class T>
void MEComponentEquation::calcVis(const T& ra, const T& dec, const T& flux, 
	const casa::Vector<double>& freq, const double u, const double v, 
	casa::Vector<T>& vreal, casa::Vector<T>& vimag) 
{
	T  delay;
	delay = casa::C::_2pi * (ra * u + dec * v)/casa::C::c;
	T phase;
	for (uint i=0;i<freq.nelements();i++) {
		phase = delay * freq(i);
		vreal(i) = flux * cos(phase);
		vimag(i) = flux * sin(phase);
	}
}

}

}
