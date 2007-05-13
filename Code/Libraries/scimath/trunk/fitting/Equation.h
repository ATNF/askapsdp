/// @file
///
/// Equation: Represent a parametrized equation. 

///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHEQUATION_H
#define SCIMATHEQUATION_H

#include <fitting/Params.h>

namespace conrad { 

namespace scimath
{

class Equation {
public:	
	/// Constructor
	/// Using default parameters
    Equation() {};
    
    /// Using specified parameters
    Equation(const Params& ip) : itsParams(ip) {};
    
    virtual ~Equation(){};

	/// Access the parameters
	const Params& parameters() const {return itsParams;};
	Params& parameters() {return itsParams;};
	
	/// Set the parameters to new values
	/// @param ip Parameters
	virtual void setParameters(const Params& ip) {itsParams=ip;};
	
	/// Check if set of parameters is valid for this equation
	/// @param ip Parameters
	virtual bool complete(const Params& ip) {return itsDefaultParams.isCongruent(ip);};
	
	/// Return a default set of parameters
	/// @param ip Parameters
	const Params& defaultParameters() const {return itsDefaultParams;};
    
protected:
	Params itsParams;
	Params itsDefaultParams;
};

}
}
#endif





