#include <fitting/Params.h>
#include <fitting/PolynomialEquation.h>
#include <fitting/NormalEquations.h>
#include <fitting/DesignMatrix.h>

#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/ArrayMath.h>
#include <scimath/Mathematics/AutoDiff.h>
#include <scimath/Mathematics/AutoDiffMath.h>

#include <cmath>

#include <stdexcept>

using conrad::scimath::NormalEquations;
using conrad::scimath::DesignMatrix;

namespace conrad
{
namespace scimath
{

void PolynomialEquation::init()
{
	// The default parameters serve as a holder for the patterns to match the actual
	// parameters. Shell pattern matching rules apply.
	itsDefaultParams.reset();
	itsDefaultParams.add("poly");
}

void PolynomialEquation::predict(const casa::Vector<double>& x, casa::Vector<double>& values) 
{
	if(parameters().isCongruent(itsDefaultParams))
	{
		throw std::invalid_argument("Parameters not consistent with this equation");
	}

    values.set(0.0);
    
	vector<string> completions(parameters().completions("poly"));
	vector<string>::iterator it;
    // Loop over all polynomials adding to the values
	for (it=completions.begin();it!=completions.end();it++) {
		string polyName("poly"+(*it));
		const casa::Vector<double> par=parameters().value(polyName);
        this->calcPoly(x, par, values);
	}
};

void PolynomialEquation::calcEquations(const casa::Vector<double>& data, const casa::Vector<double>& x, 
    DesignMatrix& designmatrix) 
{
	if(parameters().isCongruent(itsDefaultParams))
	{
		throw std::invalid_argument("Parameters not consistent with this equation");
	}

    casa::Vector<double> values(data.size());
    values=0.0;
    
    vector<string> completions(parameters().completions("poly"));
    vector<string>::iterator it;
    // Loop over all polynomials adding to the values
    for (it=completions.begin();it!=completions.end();it++) {
        string polyName("poly"+(*it));
        const casa::Vector<double> par=parameters().value(polyName);
        casa::Matrix<double> valueDerivs(data.size(), par.size());
        this->calcPoly(x, par, values);
        this->calcPolyDeriv(x, par, valueDerivs);
        designmatrix.addDerivative(polyName, valueDerivs);
    }
    casa::Vector<double> residual(data.copy());
    casa::Vector<double> weights(data.size());
    weights=1.0;
    
    residual-=values;
    designmatrix.addResidual(residual, weights);
};

void PolynomialEquation::calcPoly(const casa::Vector<double>& x, const casa::Vector<double>& parameters, 
    casa::Vector<double>& values) 
{
    for (int ix=0;ix<x.size();ix++) {
       for (int ipar=0;ipar<parameters.size();ipar++) {
           values[ix]+=parameters[ipar]*std::pow(x[ix], ipar);
       }
    }
}
void PolynomialEquation::calcPolyDeriv(const casa::Vector<double>& x, const casa::Vector<double>& parameters,
    casa::Matrix<double>& valueDerivs) 
{
    uint nPoly=parameters.size();
    uint n=x.size();
    for (int ix=0;ix<x.size();ix++) {
       for (int ipar=0;ipar<parameters.size();ipar++) {
           valueDerivs(ix,ipar)=std::pow(x[ix], ipar);
       }
    }
}

}

}
