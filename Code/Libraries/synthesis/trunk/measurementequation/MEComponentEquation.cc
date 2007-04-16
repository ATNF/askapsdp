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
	// The default parameters serve as a holder for the patterns to match the actual
	// parameters. Shell pattern matching rules apply.
	itsDefaultParams.reset();
	itsDefaultParams.add("flux.i.*");
	itsDefaultParams.add("direction.ra.*");
	itsDefaultParams.add("direction.dec.*");
}

void MEComponentEquation::predict(IDataAccessor& ida) 
{ 
	const casa::Vector<double>& freq=ida.frequency();	
	const casa::Vector<double>& time=ida.time();	
	casa::Vector<float> vis(2*freq.nelements());

	// First do the regular parameters	
	{
		vector<string> completions(parameters().completions("flux.i.*"));
		vector<string>::iterator it;
		// This outer loop is over all strings that complete the flux.i.* pattern 
		// correctly. An exception will be throw if the parameters are not
		// consistent 
		for (it=completions.begin();it!=completions.end();it++) {
		
			const double ra=parameters().value("direction.ra."+(*it))(casa::IPosition(0));
			const double dec=parameters().value("direction.dec."+(*it))(casa::IPosition(0));
			const double fluxi=parameters().value("flux.i."+(*it))(casa::IPosition(0));

			for (uint row=0;row<ida.nRow();row++) {
			
				this->calcRegularVis<float>(ra, dec, fluxi, freq, ida.uvw()(row)(0), ida.uvw()(row)(1), vis);

				for (uint i=0;i<freq.nelements();i++) {
					ida.visibility()(row,i,0) += casa::Complex(vis(2*i), vis(2*i+1));
				}
			}
		}
	}
};

void MEComponentEquation::calcEquations(IDataAccessor& ida, MEDesignMatrix& designmatrix) 
{
	const casa::Vector<double>& freq=ida.frequency();	
	const casa::Vector<double>& time=ida.time();	
	vector<string> completions(parameters().completions("flux.i.*"));
	vector<string>::iterator it;
		
	const uint nParameters=3;
	
	casa::Vector<casa::AutoDiff<double> > av(2*freq.nelements());
	for (uint i=0;i<2*freq.nelements();i++) {
		av[i]=casa::AutoDiff<double>(0.0, nParameters);
	}

	// Two values (complex) per row, channel, pol
	uint nDeriv=ida.nRow()*freq.nelements()*2*2;
	
	casa::Vector<double> raDeriv(nDeriv);
	casa::Vector<double> decDeriv(nDeriv);
	casa::Vector<double> fluxiDeriv(nDeriv);
	casa::Vector<double> residual(nDeriv);
	casa::Vector<double> weights(nDeriv);
	
		
	uint offset=0;
	for (it=completions.begin();it!=completions.end();it++) {
	
		string raName("direction.ra."+(*it));
		string decName("direction.dec."+(*it));
		string fluxName("flux.i."+(*it));

		casa::AutoDiff<double> ara(parameters().value(raName)(casa::IPosition(0)), nParameters, 0);
		casa::AutoDiff<double> adec(parameters().value(decName)(casa::IPosition(0)), nParameters, 1);
		casa::AutoDiff<double> afluxi(parameters().value(fluxName)(casa::IPosition(0)), nParameters, 2);
			
		for (uint row=0;row<ida.nRow();row++) {

			this->calcRegularVis<casa::AutoDiff<double> >(ara, adec, afluxi, freq, ida.uvw()(row)(0), ida.uvw()(row)(1), av);

			for (uint i=0;i<freq.nelements();i++) {
//				ida.visibility()(row,i,0) += casa::Complex(av(2*i).value(), av(2*i+1).value());
				residual(2*i)=av(2*i).value()-real(ida.visibility()(row,i,0));
				residual(2*i+1)=av(2*i+1).value()-imag(ida.visibility()(row,i,0));
			}

			for (uint i=0;i<2*freq.nelements();i++) {
				raDeriv(i+offset)=av[i].derivative(0);	
				decDeriv(i+offset)=av[i].derivative(1);	
				fluxiDeriv(i+offset)=av[i].derivative(2);
				weights(i+offset)=1.0;	
			}
			offset+=2*freq.nelements();
		}
		
		if(parameters().isFree(raName)) designmatrix.addDerivative(raName, raDeriv, residual, weights);
		if(parameters().isFree(decName)) designmatrix.addDerivative(decName, decDeriv, residual, weights);
		if(parameters().isFree(fluxName)) designmatrix.addDerivative(fluxName, fluxiDeriv, residual, weights);
	}
};

void MEComponentEquation::calcEquations(IDataAccessor& ida, MENormalEquations& normeq) 
{
	const casa::Vector<double>& freq=ida.frequency();	
	const casa::Vector<double>& time=ida.time();	
	vector<string> completions(parameters().completions("image.*"));
	vector<string>::iterator it;
		
	for (it=completions.begin();it!=completions.end();it++) {
	
		string imageName("image."+(*it));
			
		for (uint row=0;row<ida.nRow();row++) {

		}
		
	}
};

template<class T>
void MEComponentEquation::calcRegularVis(const T& ra, const T& dec, const T& flux, 
	const casa::Vector<double>& freq, const double u, const double v, 
	casa::Vector<T>& vis) 
{
	T  delay;
	delay = casa::C::_2pi * (ra * u + dec * v)/casa::C::c;
	T phase;
	for (uint i=0;i<freq.nelements();i++) {
		phase = delay * freq(i);
		vis(2*i)   = flux * cos(phase);
		vis(2*i+1) = flux * sin(phase);
	}
}

}

}
