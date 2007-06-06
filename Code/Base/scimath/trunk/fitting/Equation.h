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
#include <fitting/NormalEquations.h>

#include <boost/shared_ptr.hpp>

namespace conrad { 

namespace scimath
{

class Equation {
public:	
	/// Constructor
	/// Using default parameters
    Equation() {};
    
    /// Using specified parameters
    explicit Equation(const Params& ip);
    
    /// Copy constructor
    Equation(const Equation& other);
    
    /// Assignment operator
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
    
    /// Predict the data from the parameters. This changes the internal state.
    virtual void predict() {};
    
    /// Calculate the normal equations for the given data and parameters
    /// @param ne Normal equations to be filled
    virtual void calcEquations(NormalEquations& ne) {};
    
    /// Shared pointer definition
//    typedef boost::shared_ptr<Equation> ShPtr;
    typedef Equation* ShPtr;
    
    /// Clone this 
    virtual Equation::ShPtr clone();

protected:
	Params itsParams;
	Params itsDefaultParams;
};

}
}
#endif





