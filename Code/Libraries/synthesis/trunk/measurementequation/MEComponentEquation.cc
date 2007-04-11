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

	// This outer loop is over all strings that complete the flux.i.* pattern 
	// correctly. An exception will be throw if the parameters are not
	// consistent 
	vector<string> completions(parameters().regular().completions("flux.i.*"));
	vector<string>::iterator it;
	for (it=completions.begin();it!=completions.end();it++) {
	
		const double ra=parameters().regular().value("direction.ra."+(*it));
		const double dec=parameters().regular().value("direction.dec."+(*it));
		const double fluxi=parameters().regular().value("flux.i."+(*it));

		for (uint row=0;row<ida.nRow();row++) {
			
			this->calcVis<float>(ra, dec, fluxi, freq, ida.uvw()(row)(0), ida.uvw()(row)(1), vis);

			for (uint i=0;i<freq.nelements();i++) {
				real(ida.visibility()(row,i,0)) += vis(2*i);
				imag(ida.visibility()(row,i,0)) += vis(2*i+1);
			}
		}
	}
};

void MEComponentEquation::calcEquations(IDataAccessor& ida, MERegularNormalEquations& normeq) 
{
	MEDesignMatrix designmatrix;
	calcEquations(ida, designmatrix, normeq);	
}

void MEComponentEquation::calcEquations(IDataAccessor& ida, MEDesignMatrix& designmatrix, 
	MERegularNormalEquations& normeq) 
{
	const casa::Vector<double>& freq=ida.frequency();	
	const casa::Vector<double>& time=ida.time();	
	vector<string> completions(parameters().regular().completions("flux.i.*"));
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
	
	uint offset=0;
	for (it=completions.begin();it!=completions.end();it++) {
	
		string raName("direction.ra."+(*it));
		string decName("direction.dec."+(*it));
		string fluxName("flux.i."+(*it));

		casa::AutoDiff<double> ara(parameters().regular().value(raName), nParameters, 0);
		casa::AutoDiff<double> adec(parameters().regular().value(decName), nParameters, 1);
		casa::AutoDiff<double> afluxi(parameters().regular().value(fluxName), nParameters, 2);
			
		for (uint row=0;row<ida.nRow();row++) {

			this->calcVis<casa::AutoDiff<double> >(ara, adec, afluxi, freq, ida.uvw()(row)(0), ida.uvw()(row)(1), av);

			for (uint i=0;i<freq.nelements();i++) {
				real(ida.visibility()(row,i,0)) += av(2*i).value();
				imag(ida.visibility()(row,i,0)) += av(2*i+1).value();

				raDeriv(2*i+offset)=av[2*i].derivative(0);	
				decDeriv(2*i+offset)=av[2*i].derivative(1);	
				fluxiDeriv(2*i+offset)=av[2*i].derivative(2);	

				raDeriv(2*i+offset+1)=av[2*i+1].derivative(0);	
				decDeriv(2*i+offset+1)=av[2*i+1].derivative(1);	
				fluxiDeriv(2*i+offset+1)=av[2*i+1].derivative(2);	
			}
			offset+=2*freq.nelements();
		}
		
		designmatrix.addDerivative(raName, raDeriv);
		designmatrix.addDerivative(decName, decDeriv);
		designmatrix.addDerivative(fluxName, fluxiDeriv);
	}
};

template<class T>
void MEComponentEquation::calcVis(const T& ra, const T& dec, const T& flux, 
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
