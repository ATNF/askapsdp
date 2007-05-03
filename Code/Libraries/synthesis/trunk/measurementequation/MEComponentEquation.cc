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

#include <stdexcept>

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
	itsDefaultParams.add("flux.i");
	itsDefaultParams.add("direction.ra");
	itsDefaultParams.add("direction.dec");
}

void MEComponentEquation::predict(IDataAccessor& ida) 
{
	if(parameters().isCongruent(itsDefaultParams))
	{
		throw std::invalid_argument("Parameters not consistent with this equation");
	}
	const casa::Vector<double>& freq=ida.frequency();	
	const casa::Vector<double>& time=ida.time();	
	casa::Vector<float> vis(2*freq.nelements());

	vector<string> completions(parameters().completions("flux.i"));
	vector<string>::iterator it;
	// This outer loop is over all strings that complete the flux.i.* pattern 
	// correctly. An exception will be throw if the parameters are not
	// consistent 
	for (it=completions.begin();it!=completions.end();it++) {
	
		string fluxName("flux.i"+(*it));
		string raName("direction.ra"+(*it));
		string decName("direction.dec"+(*it));

		const double ra=parameters().scalarValue(raName);
		const double dec=parameters().scalarValue(decName);
		const double fluxi=parameters().scalarValue(fluxName);

		for (uint row=0;row<ida.nRow();row++) {
		
			this->calcRegularVis<float>(ra, dec, fluxi, freq, ida.uvw()(row)(0), ida.uvw()(row)(1), vis);

			for (uint i=0;i<freq.nelements();i++) {
				ida.rwVisibility()(row,i,0) += casa::Complex(vis(2*i), vis(2*i+1));
			}
		}
	}
};

void MEComponentEquation::calcEquations(IDataAccessor& ida, MEDesignMatrix& designmatrix) 
{
	if(parameters().isCongruent(itsDefaultParams))
	{
		throw std::invalid_argument("Parameters not consistent with this equation");
	}

	const casa::Vector<double>& freq=ida.frequency();	
	const casa::Vector<double>& time=ida.time();	
		
	const uint nParameters=3;
	
	// Define AutoDiff's for the output visibilities.
	casa::Vector<casa::AutoDiff<double> > av(2*freq.nelements());
	for (uint i=0;i<2*freq.nelements();i++) {
		av[i]=casa::AutoDiff<double>(0.0, nParameters);
	}

	// Set up arrays to hold the output values
	// Two values (complex) per row, channel, pol
	uint nData=ida.nRow()*freq.nelements()*2;
	casa::Vector<casa::DComplex> raDeriv(nData);
	casa::Vector<casa::DComplex> decDeriv(nData);
	casa::Vector<casa::DComplex> fluxiDeriv(nData);
	casa::Vector<casa::DComplex> residual(nData);
	casa::Vector<double> weights(nData);
	
	// Loop over all completions i.e. all sources
	vector<string> completions(parameters().completions("flux.i"));
	vector<string>::iterator it;
	for (it=completions.begin();it!=completions.end();it++) {
		uint offset=0;
	
		string raName("direction.ra"+(*it));
		string decName("direction.dec"+(*it));
		string fluxName("flux.i"+(*it));

		// Define AutoDiff's for the three unknown parameters
		casa::AutoDiff<double> ara(parameters().scalarValue(raName), nParameters, 0);
		casa::AutoDiff<double> adec(parameters().scalarValue(decName), nParameters, 1);
		casa::AutoDiff<double> afluxi(parameters().scalarValue(fluxName), nParameters, 2);
			
		for (uint row=0;row<ida.nRow();row++) {

			this->calcRegularVis<casa::AutoDiff<double> >(ara, adec, afluxi, freq, 
				ida.uvw()(row)(0), ida.uvw()(row)(1), av);

			for (uint i=0;i<freq.nelements();i++) {
				residual(i+offset)=(ida.visibility()(row,i,0))
					-casa::Complex(av(2*i).value(), av(2*i+1).value());
				raDeriv(i+offset)=casa::DComplex(av[2*i].derivative(0), av(2*i+1).derivative(0)); 	
				decDeriv(i+offset)=casa::DComplex(av[2*i].derivative(1), av(2*i+1).derivative(1)); 	
				fluxiDeriv(i+offset)=casa::DComplex(av[2*i].derivative(2), av(2*i+1).derivative(2)); 	
				weights(i+offset)=1.0;	
			}

			offset+=freq.nelements();
		}
		// Now we can add the design matrix, residual, and weights
		designmatrix.addDerivative(raName, raDeriv);
		designmatrix.addDerivative(decName, decDeriv);
		designmatrix.addDerivative(fluxName, fluxiDeriv);
		designmatrix.addResidual(residual, weights);
	}
};

void MEComponentEquation::calcEquations(IDataAccessor& ida, MENormalEquations& normeq) 
{
};
// This can be done easily by hand (and we should do for production) but I'm leaving
// it in this form for the moment to show how the differentiation is done using
// casa::AutoDiff
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
