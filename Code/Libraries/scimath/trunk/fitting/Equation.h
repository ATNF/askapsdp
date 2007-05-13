/// @file
///
/// Equation: Represent a parametrized equation. 
///
/// This is a base class. See PolynomialEquation for an example
/// of how to derive.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHEQUATION_H
#define SCIMATHEQUATION_H

#include <fitting/Params.h>
#include <fitting/DesignMatrix.h>

namespace conrad { 

namespace scimath
{

class Equation {
public:	
	/// Constructor
	/// Using default parameters
    Equation() {};
    
    /// Using specified parameters
    Equation(const Params& ip);
    
    Equation(const Equation& other);
    
    Equation& operator=(const Equation& other);
    
    virtual ~Equation();

	/// Access the parameters
	const Params& parameters() const;
	Params& parameters();
	
	/// Set the parameters to new values
	/// @param ip Parameters
	virtual void setParameters(const Params& ip);
	
	/// Check if set of parameters is valid for this equation
	/// @param ip Parameters
	virtual bool complete(const Params& ip);
	
	/// Return a default set of parameters
	/// @param ip Parameters
	const Params& defaultParameters() const;
    
    virtual void predict() = 0;
    
    virtual void calcEquations(DesignMatrix& dm) = 0;
    
protected:
	Params itsParams;
	Params itsDefaultParams;
};

}
}
#endif





