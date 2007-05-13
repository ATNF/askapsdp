/// @file
///
/// Equation: Represent a parametrized equation. 

///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHPOLYEQUATION_H
#define SCIMATHPOLYEQUATION_H

#include <fitting/Equation.h>
#include <fitting/DesignMatrix.h>
#include <fitting/NormalEquations.h>

namespace conrad { 

namespace scimath
{

class PolynomialEquation : public Equation {
public:	
	/// Constructor
	/// Using default parameters
    PolynomialEquation() : Equation() {};
    
    /// Using specified parameters
    PolynomialEquation(const Params& ip) : Equation(ip) {};
    
    virtual ~PolynomialEquation(){};
    
    virtual void predict(const casa::Vector<double>& arguments, casa::Vector<double>& values);
    
    virtual void calcEquations(const casa::Vector<double>& data, const casa::Vector<double>& arguments, DesignMatrix& dm);

protected:

    void init();

    void calcPoly(const casa::Vector<double>& arguments, const casa::Vector<double>& parameters,
        casa::Vector<double>& values);
    void calcPolyDeriv(const casa::Vector<double>& arguments, const casa::Vector<double>& parameters,
        casa::Matrix<double>& valueDerivs);

};

}
}
#endif





