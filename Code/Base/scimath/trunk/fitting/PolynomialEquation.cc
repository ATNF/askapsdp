#include <fitting/Params.h>
#include <fitting/PolynomialEquation.h>
#include <fitting/NormalEquations.h>
#include <fitting/DesignMatrix.h>

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <scimath/Mathematics/AutoDiff.h>
#include <scimath/Mathematics/AutoDiffMath.h>

#include <cmath>

using conrad::scimath::NormalEquations;
using conrad::scimath::DesignMatrix;

namespace conrad
{
  namespace scimath
  {

    PolynomialEquation::PolynomialEquation(const Params& ip,
      casa::Vector<double>& data,
      casa::Vector<double>& weights,
      casa::Vector<double>& arguments,
      casa::Vector<double>& model) : Equation(ip), itsData(data),
      itsWeights(weights), itsArguments(arguments), itsModel(model)
    {
    };

    PolynomialEquation::PolynomialEquation(casa::Vector<double>& data,
      casa::Vector<double>& weights,
      casa::Vector<double>& arguments,
      casa::Vector<double>& model) : Equation(), itsData(data),
      itsWeights(weights), itsArguments(arguments), itsModel(model)
    {
    };

    PolynomialEquation::PolynomialEquation(const PolynomialEquation& other)
    {
      operator=(other);
    }

    PolynomialEquation& PolynomialEquation::operator=(const PolynomialEquation& other)
    {
      if(this!=&other)
      {
        itsParams=other.itsParams;
        itsData=other.itsData;
        itsWeights=other.itsWeights;
        itsArguments=other.itsArguments;
        itsModel=other.itsModel;
      }
    }

    PolynomialEquation::~PolynomialEquation()
    {
    }
    
    Params PolynomialEquation::defaultParameters()
    {
      Params ip;
      ip.add("poly");
      return ip;
    }
     
    void PolynomialEquation::init()
    {
    }

    void PolynomialEquation::predict()
    {

      vector<string> completions(parameters().completions("poly"));
      if(completions.size()>0) {
// Loop over all polynomials adding to the values. 
        for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
        {
          string polyName("poly"+(*it));
          const casa::Vector<double> par(parameters().value(polyName));
          this->calcPoly(itsArguments, par, itsModel);
        }
      }
    }

    void PolynomialEquation::calcEquations(NormalEquations& ne)
    {
      vector<string> completions(parameters().completions("poly"));
      if(completions.size()>0) {
        DesignMatrix designmatrix(parameters());

        casa::Vector<double> values(itsData.size());
        values=0.0;

// Loop over all polynomials adding to the values. 
        for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
        {
          string polyName("poly"+(*it));
          const casa::Vector<double> par(parameters().value(polyName));
          casa::Matrix<double> valueDerivs(itsData.size(), par.size());
          this->calcPoly(itsArguments, par, itsModel);
          this->calcPolyDeriv(itsArguments, par, valueDerivs);
          designmatrix.addDerivative(polyName, valueDerivs);
        }
        casa::Vector<double> residual(itsData.copy());
  
        residual-=itsModel;
        designmatrix.addResidual(residual, itsWeights);
        ne.add(designmatrix);
      }
    };

    void PolynomialEquation::calcPoly(const casa::Vector<double>& x,
      const casa::Vector<double>& parameters,
      casa::Vector<double>& values)
    {
      for (int ix=0;ix<x.size();ix++)
      {
        for (int ipar=0;ipar<parameters.size();ipar++)
        {
          /// @todo Optimize calculation of values
          values[ix]+=parameters[ipar]*std::pow(x[ix], ipar);
        }
      }
    }
    void PolynomialEquation::calcPolyDeriv(const casa::Vector<double>& x,
      const casa::Vector<double>& parameters,
      casa::Matrix<double>& valueDerivs)
    {
      uint nPoly=parameters.size();
      uint n=x.size();
      for (int ix=0;ix<x.size();ix++)
      {
        for (int ipar=0;ipar<parameters.size();ipar++)
        {
          /// @todo Optimize calculation of derivatives
          valueDerivs(ix,ipar)=std::pow(x[ix], ipar);
        }
      }
    }

    Equation::ShPtr PolynomialEquation::clone() const
    {
      return Equation::ShPtr(new PolynomialEquation(*this));
    }

  }

}
