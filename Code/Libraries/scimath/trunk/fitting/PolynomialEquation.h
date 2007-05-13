/// @file
///
/// PolynomialEquation: Represent a polynomial equation. 
/// The parameters of the polynomial are supplied via the
/// Params class. The data and arguments are also part
/// of the constructor. 
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
    PolynomialEquation() : Equation() {};
    
    /// Constructor for real use
    /// @param ip Coefficients of polynomial stored with names poly.*
    /// @param data Data constraints
    /// @param arguments Arguments for the polynomial
    /// @param model Model (to be calculated)
    PolynomialEquation(const Params& ip, casa::Vector<double>& data, 
        casa::Vector<double>& arguments, casa::Vector<double>& model);
        
    /// Copy constructor
    PolynomialEquation(const PolynomialEquation& other);
    
    /// Assignment operator
    PolynomialEquation& operator=(const PolynomialEquation& other);
    
    /// Destructor
    virtual ~PolynomialEquation(){};
    
    /// Predict the model data
    virtual void predict();
    
    /// Calculate the design matrix
    /// @param dm Design matrix
    virtual void calcEquations(DesignMatrix& dm);

protected:

    void init();

    void calcPoly(const casa::Vector<double>& arguments, const casa::Vector<double>& parameters,
        casa::Vector<double>& values);
    void calcPolyDeriv(const casa::Vector<double>& arguments, const casa::Vector<double>& parameters,
        casa::Matrix<double>& valueDerivs);
        
    casa::Vector<double> itsData;
    casa::Vector<double> itsArguments;
    casa::Vector<double> itsModel;

};

}
}
#endif





